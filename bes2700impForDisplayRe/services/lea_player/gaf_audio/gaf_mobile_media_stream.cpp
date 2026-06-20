/**
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
#include "bluetooth_bt_api.h"
#include "hal_dma.h"
#include "hal_trace.h"
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
#include "besbt_string.h"

#include "gaf_media_sync.h"
#include "gaf_media_pid.h"
#include "gaf_media_common.h"
#include "gaf_codec_lc3.h"
#include "gaf_mobile_media_stream.h"
#include "gaf_media_sync.h"
#include "gaf_stream_dbg.h"
#include "gaf_media_stream.h"

#include "ble_audio_mobile_info.h"
#ifdef BLE_USB_AUDIO_SUPPORT
#include "gaf_mobile_usb_audio.h"
#endif

#ifdef GAF_CODEC_CROSS_CORE
#include "app_dsp_m55.h"
#include "mcu_dsp_m55_app.h"
#include "gaf_codec_cc_common.h"
#include "gaf_codec_cc_bth.h"
#endif

#ifdef GAF_DSP
#include "dsp_loader.h"
#endif

#include "rwble_config.h"
#include "bes_aob_api.h"

#include "app_bt_sync.h"

#ifdef AOB_MOBILE_ENABLED
/*********************external function declaration*************************/
extern "C" uint32_t btdrv_reg_op_cig_anchor_timestamp(uint8_t link_id);

#ifdef BLE_USB_AUDIO_SUPPORT
extern void app_usbaudio_entry(void);
extern "C"  bool app_usbaudio_mode_on(void);
#endif

#ifdef GAF_CODEC_CROSS_CORE
GAF_AUDIO_STREAM_ENV_T* pLocalMobileStreamEnvPtr;
#endif

/************************private macro defination***************************/
#ifndef BT_AUDIO_CACHE_2_UNCACHE
#define BT_AUDIO_CACHE_2_UNCACHE(addr) \
                    ((unsigned char *)((unsigned int)addr & ~(0x04000000)))
#endif

/************************private variable defination************************/
GAF_AUDIO_STREAM_ENV_T gaf_mobile_audio_stream_env;
osMutexDef(gaf_mobile_decoder_buffer_mutex);
osMutexDef(gaf_mobile_encoder_buffer_mutex);

#ifdef GAF_CODEC_CROSS_CORE
osMutexDef(gaf_m55_encoder_buffer_mutex);
#endif

static GAF_MEDIA_DWELLING_INFO_T gaf_cis_mobile_media_dwelling_info[GAF_MOB_MAXIMUM_CONNECTION_COUNT];

#ifdef BLE_USB_AUDIO_SUPPORT
const GAF_MEDIA_STREAM_TYPE_OPERATION_RULE_T gaf_cis_mobile_stream_type_op_rule =
{
#if defined(BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
    GAF_AUDIO_STREAM_TYPE_FLEXIBLE, GAF_AUDIO_TRIGGER_BY_ISOLATE_STREAM, 1, 1,
#else
    GAF_AUDIO_STREAM_TYPE_FLEXIBLE, GAF_AUDIO_TRIGGER_BY_ISOLATE_STREAM, 2, 2,
#endif
};
#endif

const GAF_MEDIA_STREAM_TYPE_OPERATION_RULE_T gaf_cis_mobile_stream_type_op_rule_all =
{
    GAF_AUDIO_STREAM_TYPE_FLEXIBLE, GAF_AUDIO_TRIGGER_BY_ISOLATE_STREAM, 2, 2,
};

#ifdef AOB_UC_TEST
uint8_t mobile_freq;
#endif

/*******************************GAF CUSTOM**************************************/
static const GAF_STREAM_COMMON_CUSTOM_DATA_HANDLER_FUNC_T *gaf_uc_cli_custom_data_callback = NULL;
/****************************function defination****************************/
static uint8_t gaf_mobile_get_connected_device_count(void)
{
    uint8_t gaf_connected_dev_cnt = 0;
    for (uint8_t con_lid = 0; con_lid < GAF_MOB_MAXIMUM_CONNECTION_COUNT; con_lid++)
    {
        for (uint8_t idx = 0; idx < GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT; idx++)
        {
            if (gaf_cis_mobile_media_dwelling_info[con_lid].playback_ase_id[idx]
                != GAF_INVALID_ASE_INDEX)
            {
                gaf_connected_dev_cnt += 1;
                break;
            }
        }
    }

    return gaf_connected_dev_cnt;
}

static void gaf_mobile_audio_add_ase_into_playback_ase_list(uint8_t con_lid, uint8_t ase_lid)
{
    LOG_D("set con_lid %d playback ase lid %d", con_lid, ase_lid);
    uint8_t idx = 0;
    if (ase_lid < GAF_AUDIO_ASE_TOTAL_COUNT)
    {
        if (GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT ==
            gaf_stream_common_get_ase_idx_in_ase_lid_list(
                &gaf_cis_mobile_media_dwelling_info[con_lid].playback_ase_id[0], ase_lid))
        {
            idx = gaf_stream_common_get_valid_idx_in_ase_lid_list(
                    &gaf_cis_mobile_media_dwelling_info[con_lid].playback_ase_id[0]);
            if (idx == GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT)
            {
                LOG_E("no more space for adding playback ase lid: %d into list!!!", ase_lid);
            }
            else
            {
                gaf_cis_mobile_media_dwelling_info[con_lid].playback_ase_id[idx] = ase_lid;
            }
        }
        else
        {
            LOG_E("ase_lid: %d already in playback ase list!!!", ase_lid);
        }
    }
    else
    {
        LOG_E("no more space for adding playback ase list!!!");
    }
}

static void gaf_mobile_audio_remove_ase_from_playback_ase_list(uint8_t con_lid, uint8_t ase_lid)
{
    LOG_D("set con_lid %d playback ase lid %d", con_lid, ase_lid);
    uint8_t idx = 0;
    if (ase_lid < GAF_AUDIO_ASE_TOTAL_COUNT)
    {
        idx = gaf_stream_common_get_ase_idx_in_ase_lid_list(
                &gaf_cis_mobile_media_dwelling_info[con_lid].playback_ase_id[0], ase_lid);
        if (idx == GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT)
        {
            LOG_E("ase lid: %d is not in the list!!!", ase_lid);
            return;
        }
        gaf_cis_mobile_media_dwelling_info[con_lid].playback_ase_id[idx] = GAF_INVALID_ASE_INDEX;
    }
    else
    {
        LOG_E("no more space for removing playback ase list!!!");
    }
}

static void gaf_mobile_audio_add_ase_into_capture_ase_list(uint8_t con_lid, uint8_t ase_lid)
{
    LOG_D("set con_lid %d capture ase lid %d", con_lid, ase_lid);
    uint8_t idx = 0;
    if (ase_lid < GAF_AUDIO_ASE_TOTAL_COUNT)
    {
        if (GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT ==
            gaf_stream_common_get_ase_idx_in_ase_lid_list(
                &gaf_cis_mobile_media_dwelling_info[con_lid].capture_ase_id[0], ase_lid))
        {
            idx = gaf_stream_common_get_valid_idx_in_ase_lid_list(
                    &gaf_cis_mobile_media_dwelling_info[con_lid].capture_ase_id[0]);
            if (idx == GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT)
            {
                LOG_E("no more space for adding capture ase lid: %d into list!!!", ase_lid);
            }
            else
            {
                gaf_cis_mobile_media_dwelling_info[con_lid].capture_ase_id[idx] = ase_lid;
            }
        }
        else
        {
            LOG_E("ase_lid: %d already in capture ase list!!!", ase_lid);
        }
    }
    else
    {
        LOG_E("no more space for adding capture ase list!!!");
    }
}

static void gaf_mobile_audio_remove_ase_from_capture_ase_list(uint8_t con_lid, uint8_t ase_lid)
{
    LOG_D("set con_lid %d capture ase lid %d", con_lid, ase_lid);
    uint8_t idx = 0;
    if (ase_lid < GAF_AUDIO_ASE_TOTAL_COUNT)
    {
        idx = gaf_stream_common_get_ase_idx_in_ase_lid_list(
                &gaf_cis_mobile_media_dwelling_info[con_lid].capture_ase_id[0], ase_lid);
        if (idx == GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT)
        {
            LOG_E("ase lid: %d is not in the list!!!", ase_lid);
            return;
        }
        gaf_cis_mobile_media_dwelling_info[con_lid].capture_ase_id[idx] = GAF_INVALID_ASE_INDEX;
    }
    else
    {
        LOG_E("no more space for removing capture ase list!!!");
    }
}

static uint8_t* gaf_mobile_audio_get_playback_ase_index_list(uint8_t con_lid)
{
    LOG_D("get playback con_lid %d", con_lid);
    return &gaf_cis_mobile_media_dwelling_info[con_lid].playback_ase_id[0];
}

static uint8_t* gaf_mobile_audio_get_capture_ase_index_list(uint8_t con_lid)
{
    LOG_D("get capture con_lid %d", con_lid);
    return &gaf_cis_mobile_media_dwelling_info[con_lid].capture_ase_id[0];
}

static void gaf_mobile_audio_clear_playback_ase_index_list(uint8_t con_lid)
{
    LOG_D("clear playback ase list con_lid %d", con_lid);
    memset_s(&gaf_cis_mobile_media_dwelling_info[con_lid].playback_ase_id[0],
        GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT*sizeof(uint8_t), GAF_INVALID_ASE_INDEX,
        GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT*sizeof(uint8_t));
}

static void gaf_mobile_audio_clear_capture_ase_index_list(uint8_t con_lid)
{
    LOG_D("clear capture ase list con_lid %d", con_lid);
    memset_s(&gaf_cis_mobile_media_dwelling_info[con_lid].capture_ase_id[0],
        GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT*sizeof(uint8_t), GAF_INVALID_ASE_INDEX,
        GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT*sizeof(uint8_t));
}

static gaf_stream_buff_list_t * gaf_mobile_audio_get_playback_buf_list_by_cis_channel(GAF_AUDIO_STREAM_ENV_T* _pStreamEnv, uint8_t cisChannel)
{
    LOG_D("%s %d", __func__, cisChannel);
    for (uint8_t i = 0; i < gaf_cis_mobile_stream_type_op_rule_all.playback_ase_count; i++)
    {
        if (_pStreamEnv->stream_context.playback_buff_list[i].cisChannel == cisChannel)
        {
            return &_pStreamEnv->stream_context.playback_buff_list[i].buff_list;
        }
    }
    ASSERT(0, "gaf_mobile_audio_get_playback_buf_list:%d %d %d",
        _pStreamEnv->stream_context.playback_buff_list[0].cisChannel,
        _pStreamEnv->stream_context.playback_buff_list[1].cisChannel, cisChannel);
    return NULL;
}

gaf_media_data_t *gaf_mobile_audio_get_packet(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
    uint32_t dmaIrqHappeningTimeUs, uint8_t cisChannel)
{
    bool isPacketValid = false;
    gaf_stream_buff_list_t *list = gaf_mobile_audio_get_playback_buf_list_by_cis_channel(pStreamEnv, cisChannel);
    gaf_media_data_t *decoder_frame_p = NULL;
    gaf_media_data_t *decoder_frame_read = NULL;
    uint16_t expectedSeq = 0;
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
    if (true == is_support_ble_audio_mobile_m55_decode)
    {
        if (pStreamEnv->stream_context.right_cis_channel == cisChannel)
        {
            expectedSeq = pStreamEnv->stream_context.lastestPlaybackSeqNumR;
        }
        else if (pStreamEnv->stream_context.left_cis_channel == cisChannel)
        {
            expectedSeq = pStreamEnv->stream_context.lastestPlaybackSeqNumL;
        }
    }
    else
    {
        expectedSeq = pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX];
    }
#else
    expectedSeq = pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX];
#endif
    do
    {
        list->node = gaf_list_begin(list);
        if (NULL == list->node)
        {
            LOG_W("gaf playback list is empty.");
            break;
        }
        decoder_frame_read = (gaf_media_data_t *)gaf_list_node(list);
        LOG_D("%s pkt_seq_nb 0x%x expectedSeq 0x%x pkt_status 0x%x channel %d", __func__,
            decoder_frame_read->pkt_seq_nb, expectedSeq, decoder_frame_read->pkt_status,
            cisChannel);
#ifndef GAF_MOBILE_TEMP_SOLUTION_ENABLE
        if ((decoder_frame_read->pkt_seq_nb == expectedSeq) &&
            (GAF_ISO_PKT_STATUS_VALID == decoder_frame_read->pkt_status))
#else
        if (true) //GAF_ISO_PKT_STATUS_VALID == decoder_frame_read->pkt_status)
#endif
        {
            int32_t bt_time_diff = 0;
            bt_time_diff = GAF_AUDIO_CLK_32_BIT_DIFF(decoder_frame_read->time_stamp, dmaIrqHappeningTimeUs);
            // check the time-stamp
            if (GAF_MOBILE_AUDIO_MAX_DIFF_BT_TIME < (int32_t)(bt_time_diff - GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US))
            {
                LOG_I("received packet's playtime has passed.");
                LOG_I("Seq: 0x%x Local time: %d packet's time stampt: %d time diff: %d",
                    decoder_frame_read->pkt_seq_nb,
                    dmaIrqHappeningTimeUs,
                    decoder_frame_read->time_stamp,
                    (int32_t)(bt_time_diff - GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US));
                gaf_list_remove_generic(list, decoder_frame_read);
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
                if (true == is_support_ble_audio_mobile_m55_decode)
                {
                    if (pStreamEnv->stream_context.right_cis_channel == cisChannel)
                    {
                        pStreamEnv->stream_context.lastestPlaybackSeqNumR++;
                        expectedSeq = pStreamEnv->stream_context.lastestPlaybackSeqNumR++;
                    }
                    else if (pStreamEnv->stream_context.left_cis_channel == cisChannel)
                    {
                        pStreamEnv->stream_context.lastestPlaybackSeqNumL++;
                        expectedSeq = pStreamEnv->stream_context.lastestPlaybackSeqNumL++;
                    }
                }
                else
                {
                    pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]++;
                    expectedSeq = pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX];
                }
#else
                    pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]++;
                    expectedSeq = pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX];
#endif
                continue;
            }
            else if (GAF_MOBILE_AUDIO_MAX_DIFF_BT_TIME < (int32_t)(GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US - bt_time_diff))
            {
                LOG_I("received packet's playtime hasn't arrived.");
                LOG_I("Seq: 0x%x Local time: %d packet's time stampt: %d time diff: %d",
                    decoder_frame_read->pkt_seq_nb,
                    dmaIrqHappeningTimeUs,
                    decoder_frame_read->time_stamp,
                    (int32_t)(GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US - bt_time_diff));
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
                if (true == is_support_ble_audio_mobile_m55_decode)
                {
                    if (pStreamEnv->stream_context.right_cis_channel == cisChannel)
                    {
                        pStreamEnv->stream_context.lastestPlaybackSeqNumR = decoder_frame_read->pkt_seq_nb-1;
                    }
                    else if (pStreamEnv->stream_context.left_cis_channel == cisChannel)
                    {
                        pStreamEnv->stream_context.lastestPlaybackSeqNumL = decoder_frame_read->pkt_seq_nb-1;
                    }
                }
                else
                {
                    pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX] = decoder_frame_read->pkt_seq_nb-1;
                }
#else
                    pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX] = decoder_frame_read->pkt_seq_nb-1;
#endif
            }
            else
            {
                if (GAF_ISO_PKT_STATUS_VALID == decoder_frame_read->pkt_status)
                {
                    isPacketValid = true;
                }
                else
                {
                    gaf_list_remove_generic(list, decoder_frame_read);
                }
            }
        }
        else
        {
        #ifndef GAF_MOBILE_TEMP_SOLUTION_ENABLE
            if (decoder_frame_read->pkt_seq_nb != expectedSeq)
        #else
            if (true)
        #endif
            {
                int32_t bt_time_diff = 0;
                bt_time_diff = GAF_AUDIO_CLK_32_BIT_DIFF(decoder_frame_read->time_stamp, dmaIrqHappeningTimeUs);
                bt_time_diff = GAF_AUDIO_CLK_32_BIT_DIFF(bt_time_diff, GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US);
                bt_time_diff = GAF_AUDIO_ABS(bt_time_diff);
                if (bt_time_diff < GAF_MOBILE_AUDIO_MAX_DIFF_BT_TIME)
                {
                    expectedSeq = decoder_frame_read->pkt_seq_nb;
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
                    if (true == is_support_ble_audio_mobile_m55_decode)
                    {
                        if (pStreamEnv->stream_context.right_cis_channel == cisChannel)
                        {
                            pStreamEnv->stream_context.lastestPlaybackSeqNumR = expectedSeq;
                        }
                        else if (pStreamEnv->stream_context.left_cis_channel == cisChannel)
                        {
                            pStreamEnv->stream_context.lastestPlaybackSeqNumL = expectedSeq;
                        }
                    }
                    else
                    {
                        pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX] = expectedSeq;
                    }
#else
                        pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX] = expectedSeq;
#endif
                    LOG_I("Get frame with right ts %d local ts %d",
                        decoder_frame_read->time_stamp, dmaIrqHappeningTimeUs);
                    continue;
                }
                LOG_I("Seq num error: expected 0x%x actual 0x%x",
                    expectedSeq, decoder_frame_read->pkt_seq_nb);
                if (expectedSeq > 0)
                {
                    if (decoder_frame_read->pkt_seq_nb < expectedSeq)
                    {
                        gaf_list_remove_generic(list, decoder_frame_read);
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    if (decoder_frame_read->pkt_seq_nb > 0xFF00)
                    {
                        gaf_list_remove_generic(list, decoder_frame_read);
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else if (GAF_ISO_PKT_STATUS_VALID != decoder_frame_read->pkt_status)
            {
                LOG_I("packet is invalid, status %d", decoder_frame_read->pkt_status);
                // for those valid seq number but invalid content packets, consume them
                gaf_list_remove_generic(list, decoder_frame_read);
            }
        }
        break;
    } while (1);
    if (isPacketValid)
    {
        decoder_frame_p = decoder_frame_read;
        decoder_frame_p->isPLC = false;
        gaf_list_only_remove_node(list, decoder_frame_read);
    }
    else
    {
        LOG_I("Hit PLC event!");
        decoder_frame_p = (gaf_media_data_t *)gaf_stream_data_frame_malloc(0);
        decoder_frame_p->time_stamp = dmaIrqHappeningTimeUs - GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US;
        if (decoder_frame_read)
        {
            decoder_frame_p->pkt_seq_nb = decoder_frame_read->pkt_seq_nb;
        }
        else
        {
            decoder_frame_p->pkt_seq_nb = 0;
        }
        decoder_frame_p->pkt_status = GAF_ISO_PKT_STATUS_INVALID;
        decoder_frame_p->data_len = 0;
        decoder_frame_p->isPLC = true;
    }
    return decoder_frame_p;
}
static gaf_media_data_t* gaf_mobile_audio_store_received_packet(void* _pStreamEnv, gaf_media_data_t *header, uint8_t cisChannel)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    gaf_stream_buff_list_t *list = gaf_mobile_audio_get_playback_buf_list_by_cis_channel(pStreamEnv, cisChannel);
    if (gaf_list_length(list) < pStreamEnv->stream_info.playbackInfo.maxCachedEncodedAudioPacketCount)
    {
        gaf_media_data_t *decoder_frame_p = header;
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
        if (true == is_support_ble_audio_mobile_m55_decode)
        {
            decoder_frame_p->cisChannel = cisChannel;
        }
#endif

        gaf_list_append(list, decoder_frame_p);
        LOG_D("received iso data: data_len %d list len updated to %d seq 0x%x timestamp %d",
             header->data_len, gaf_list_length(list), header->pkt_seq_nb, header->time_stamp);
        return decoder_frame_p;
    }
    else
    {
        LOG_W("%s list full current list_len:%d data_len:%d", __func__, gaf_list_length(list), header->data_len);
        return NULL;
    }
}
static GAF_AUDIO_STREAM_ENV_T* gaf_mobile_audio_get_stream_env_by_chl_index(uint8_t channel)
{
    for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++) {
        if (gaf_mobile_audio_stream_env.stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl == channel)
        {
            return &gaf_mobile_audio_stream_env;
        }
    }
    //ASSERT(0, "Receive a CIS packet before cooresponding stream is ready %d!", channel);
    return NULL;
}
void gaf_mobile_audio_receive_data(uint16_t conhdl, GAF_ISO_PKT_STATUS_E pkt_status)
{
    uint8_t channel = BLE_ISOHDL_TO_ACTID(conhdl);
    // map to gaf stream context
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = gaf_mobile_audio_get_stream_env_by_chl_index(channel);
#ifdef GAF_CODEC_CROSS_CORE
#ifdef AOB_MOBILE_ENABLED
        pLocalMobileStreamEnvPtr = pStreamEnv;
#endif
#endif
    uint32_t current_bt_time = 0;
    uint32_t trigger_bt_time = 0;
    gaf_media_data_t decoder_frame_info;
    gaf_media_data_t* p_decoder_frame = NULL;
    gaf_media_data_t* storedFramePointer = NULL;
    if (pStreamEnv == NULL) {
        while ((p_decoder_frame = (gaf_media_data_t *)bes_ble_bap_dp_itf_get_rx_data(conhdl, NULL)))
        {
            gaf_stream_data_free(p_decoder_frame);
        }
        return;
    }

    while ((p_decoder_frame = (gaf_media_data_t *)bes_ble_bap_dp_itf_get_rx_data(conhdl, NULL)))
    {
        ASSERT(p_decoder_frame->data_len <= pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize,
            "%s len %d %d, channel:%d, playbackInfo:%p", __func__, p_decoder_frame->data_len,
            pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize, channel, &(pStreamEnv->stream_info.playbackInfo));
        LOG_I("%s pkt_status %d pkt_seq_nb 0x%x channel %d, len %d", __func__,
            pkt_status, p_decoder_frame->pkt_seq_nb, channel, p_decoder_frame->data_len);

        if ((pStreamEnv->stream_context.playback_stream_state >=
            GAF_PLAYBACK_STREAM_START_TRIGGERING) ||
            ((GAF_ISO_PKT_STATUS_VALID == p_decoder_frame->pkt_status)
            &&(p_decoder_frame->data_len > 0)))
        {
            if (gaf_uc_cli_custom_data_callback->encoded_packet_recv_cb)
            {
                gaf_uc_cli_custom_data_callback->encoded_packet_recv_cb(p_decoder_frame);
            }
            storedFramePointer = gaf_mobile_audio_store_received_packet(pStreamEnv, p_decoder_frame, channel);
        }

        decoder_frame_info = *p_decoder_frame;
        if (storedFramePointer == NULL)
        {
            gaf_stream_data_free(p_decoder_frame);
        }

#ifdef GAF_DECODER_CROSS_CORE_USE_M55
        if (true == is_support_ble_audio_mobile_m55_decode)
        {
            if ((pStreamEnv->stream_context.right_cis_channel == 0) || (pStreamEnv->stream_context.left_cis_channel == 0))
            {
                if (pStreamEnv->stream_context.right_cis_channel == 0)
                {
                    pStreamEnv->stream_context.right_cis_channel = channel;
                }
                else if (pStreamEnv->stream_context.left_cis_channel == 0 && channel != pStreamEnv->stream_context.right_cis_channel)
                {
                    pStreamEnv->stream_context.left_cis_channel = channel;
                }
            }
            decoder_frame_info.cisChannel     = channel;
        }
#endif

        if (((GAF_ISO_PKT_STATUS_VALID == decoder_frame_info.pkt_status) &&
            (decoder_frame_info.data_len > 0)) &&
            (pStreamEnv->stream_context.playback_stream_state < GAF_PLAYBACK_STREAM_START_TRIGGERING))
        {
            if ((GAF_AUDIO_TRIGGER_BY_ISOLATE_STREAM ==
                gaf_cis_mobile_stream_type_op_rule_all.trigger_stream_type ) ||
                (GAF_AUDIO_TRIGGER_BY_PLAYBACK_STREAM == gaf_cis_mobile_stream_type_op_rule_all.trigger_stream_type))
            {
                if (GAF_PLAYBACK_STREAM_INITIALIZED == pStreamEnv->stream_context.playback_stream_state)
                {
                    gaf_stream_buff_list_t *list =
                            gaf_mobile_audio_get_playback_buf_list_by_cis_channel(pStreamEnv, channel);
                    gaf_media_data_t *decoder_frame_read = NULL;
                    // Loop find valid packet
                    do
                    {
                        list->node = gaf_list_begin(list);
                        if (NULL == list->node)
                        {
                            break;
                        }
                        decoder_frame_read = (gaf_media_data_t *)gaf_list_node(list);

                        current_bt_time = gaf_media_sync_get_curr_time();
                        LOG_I("%s expected play us %u current us %u seq 0x%x", __func__,
                            decoder_frame_read->time_stamp, current_bt_time, decoder_frame_read->pkt_seq_nb);
                        trigger_bt_time = decoder_frame_read->time_stamp + GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US -
                            (uint32_t)(pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs);
                        LOG_I("calculated trigger ticks %u", trigger_bt_time);
                        if (current_bt_time < trigger_bt_time)
                        {
                            LOG_I("Starting playback seq num 0x%x", decoder_frame_read->pkt_seq_nb);
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
                            if (true == is_support_ble_audio_mobile_m55_decode)
                            {
                                if (pStreamEnv->stream_context.right_cis_channel == channel)
                                {
                                    pStreamEnv->stream_context.lastestPlaybackSeqNumR = decoder_frame_read->pkt_seq_nb;
                                }
                                else if (pStreamEnv->stream_context.left_cis_channel == channel)
                                {
                                    pStreamEnv->stream_context.lastestPlaybackSeqNumL = decoder_frame_read->pkt_seq_nb;
                                }
                            }
                            else
                            {
                                pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX] = decoder_frame_read->pkt_seq_nb;
                            }
#else
                                pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX] = decoder_frame_read->pkt_seq_nb;
#endif
#ifndef BLE_USB_AUDIO_SUPPORT
                            if (GAF_AUDIO_TRIGGER_BY_ISOLATE_STREAM == gaf_cis_mobile_stream_type_op_rule_all.trigger_stream_type)
                            {
                                gaf_media_prepare_playback_trigger(pStreamEnv->stream_context.playbackTriggerChannel);
                                af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
                            }
                            gaf_stream_common_set_playback_trigger_time(pStreamEnv, trigger_bt_time);
#else
                            af_stream_start(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
                            gaf_stream_common_set_playback_trigger_time_generic(pStreamEnv, AUD_STREAM_CAPTURE,
                                                        trigger_bt_time);
#endif
                            break;
                        }
                        else
                        {
                            LOG_I("time_stamp error");
                            gaf_list_remove(list, decoder_frame_read);
                        }
                    } while (1);
                }
            }
        }
    }
}

static void gaf_mobile_audio_process_pcm_data_send(void *pStreamEnv_,void *payload_,
    uint32_t payload_size, uint32_t ref_time)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T* )pStreamEnv_;
    uint8_t *payload = (uint8_t *)payload_;

    uint32_t payload_len_per_channel = 0;
    uint32_t channel_shift = 0;
    uint32_t audio_allocation_bf = 0;
    uint8_t audio_allocation_cnt = 0;
    bool stereo_channel_support = false;
    bool is_right_channel = false;
    /// @see pCommonInfo->num_channels in SINK
    ASSERT(pStreamEnv->stream_info.captureInfo.num_channels == AUD_CHANNEL_NUM_2, "need stereo channel here");
    // Check all ase for streaming send
    for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
    {
        if (pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl == GAF_AUDIO_INVALID_ISO_CHANNEL)
        {
            continue;
        }
        channel_shift = 0;
        stereo_channel_support = bes_ble_bap_pacc_is_peer_support_stereo_channel(pStreamEnv->stream_info.con_lid,
                                                                                                    BES_BLE_GAF_DIRECTION_SINK);
        audio_allocation_bf = pStreamEnv->stream_info.captureInfo.aseChInfo[i].allocation_bf;
        is_right_channel = ((audio_allocation_bf & (BES_BLE_LOC_SIDE_RIGHT | BES_BLE_LOC_FRONT_RIGHT)) != 0);
        audio_allocation_cnt = bes_ble_audio_get_location_fs_l_r_cnt(audio_allocation_bf);

        // Config palyload len  for AUD_CHANNEL_NUM_2
        payload_len_per_channel = payload_size / AUD_CHANNEL_NUM_2;
        // Most of time, pStreamEnv->stream_info.captureInfo.num_channels == AUD_CHANNEL_NUM_2
        if (!stereo_channel_support)
        {
            // Only one channel in this ASE, choose one pcm channel
            if (is_right_channel)
            {
                // shift to right channel
                channel_shift += payload_len_per_channel;
            }
        }
        else// if STEREO CHANNNEL
        {
            //Check is there use two ASE
            if (audio_allocation_cnt == AUD_CHANNEL_NUM_1 && is_right_channel)
            {
                // shift to right channel
                channel_shift += payload_len_per_channel;
            }
            else if (audio_allocation_cnt == AUD_CHANNEL_NUM_2)
            {
                payload_len_per_channel = payload_size;
            }
        }
        /// gaf custom, maybe a data watch point, add some pattern in packet before send
        if (gaf_uc_cli_custom_data_callback->encoded_packet_send_cb)
        {
            gaf_uc_cli_custom_data_callback->encoded_packet_send_cb(payload + channel_shift, payload_len_per_channel);
        }
        LOG_D("[CAPTURE SEND] p_len:%d stereo supp: %d, allocation_bf: 0x%x, shift :%d",
              payload_len_per_channel, stereo_channel_support, audio_allocation_bf, channel_shift);
        bes_ble_bap_iso_dp_send_data(pStreamEnv->stream_info.captureInfo.aseChInfo[i].cis_hdl,
                                pStreamEnv->stream_context.latestCaptureSeqNum,
                                payload + channel_shift, payload_len_per_channel,
                                ref_time);
    }
}

static void gaf_mobile_audio_process_pcm_data(GAF_AUDIO_STREAM_ENV_T *_pStreamEnv, uint8_t *ptrBuf, uint32_t length)
{
#ifdef GAF_CODEC_CROSS_CORE
    pLocalMobileCaptureStreamEnvPtr = _pStreamEnv;
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
    // it's possible the multiple DMA irq triggered message accumulates,
    // so the acuiqred dmaIrqHappeningTimeUs is the last dma irq, which has
    // been handled. For this case, ignore this dma irq message handling
    if ((!_pStreamEnv) ||
        (GAF_CAPTURE_STREAM_START_TRIGGERING > _pStreamEnv->stream_context.capture_stream_state) ||
        ((GAF_CAPTURE_STREAM_STREAMING_TRIGGERED == _pStreamEnv->stream_context.capture_stream_state) &&
        (dmaIrqHappeningTimeUs == _pStreamEnv->stream_context.lastCaptureDmaIrqTimeUs)))
    {
        LOG_W("accumulated irq messages happen!");
        memset(ptrBuf, 0x0, length);
        return;
    }
    if (GAF_CAPTURE_STREAM_STREAMING_TRIGGERED != _pStreamEnv->stream_context.capture_stream_state)
    {
        gaf_stream_common_update_capture_stream_state(_pStreamEnv,
                                                     GAF_CAPTURE_STREAM_STREAMING_TRIGGERED);
        gaf_stream_common_clr_trigger(_pStreamEnv->stream_context.captureTriggerChannel);
    }
    gaf_stream_common_capture_timestamp_checker(_pStreamEnv, dmaIrqHappeningTimeUs);
    dmaIrqHappeningTimeUs += (uint32_t)_pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs;
    LOG_D("length %d encoded_len %d filled timestamp %u", length,
        _pStreamEnv->stream_info.captureInfo.encoded_frame_size,
        dmaIrqHappeningTimeUs);

    /// gaf custom, may be a watch point to add some pattern in pcm data to be encoded
    if (gaf_uc_cli_custom_data_callback->raw_pcm_data_cb)
    {
        gaf_uc_cli_custom_data_callback->raw_pcm_data_cb(ptrBuf, length);
    }

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
    do {
peek_again:
        if (gaf_m55_deinit_status.capture_deinit == true)
        {
            break;
        }
        bool is_accessed = false;
        is_accessed = gaf_stream_common_store_received_pcm_packet((void *)_pStreamEnv, dmaIrqHappeningTimeUs, ptrBuf, length);
        if (_pStreamEnv->stream_context.isUpStreamingStarted)
        {
            uint8_t capture_list_length = 0;
            capture_list_length = gaf_list_length(&_pStreamEnv->stream_context.m55_capture_buff_list.buff_list);
            if ((false == is_accessed) && (capture_list_length > GAF_ENCODER_PCM_DATA_BUFF_LIST_MAX_LENGTH)) {
                goto peek_again;
            }
            // malloc output_buf to cached encoded data
            uint8_t *output_buf = NULL;
            uint32_t lc3_encoded_frame_len = (uint32_t)(_pStreamEnv->stream_info.captureInfo.encoded_frame_size);
            int32_t channelNum = _pStreamEnv->stream_info.captureInfo.num_channels;
            output_buf = (uint8_t *)gaf_stream_heap_cmalloc(lc3_encoded_frame_len*channelNum);
            // bth fetch encoded data
            gaf_encoder_core_fetch_encoded_data(_pStreamEnv, output_buf, lc3_encoded_frame_len*channelNum, dmaIrqHappeningTimeUs);
            // bth send out encoded data
            gaf_mobile_audio_process_pcm_data_send(_pStreamEnv,output_buf, lc3_encoded_frame_len*channelNum, dmaIrqHappeningTimeUs);
            gaf_stream_heap_free(output_buf);
        }
    } while(0);
#else
    _pStreamEnv->func_list->encoder_func_list->encoder_encode_frame_func(_pStreamEnv, dmaIrqHappeningTimeUs,
        length, ptrBuf,&_pStreamEnv->stream_context.codec_alg_context[0],
        &gaf_mobile_audio_process_pcm_data_send);
#endif
}

static void gaf_mobile_audio_process_encoded_data(GAF_AUDIO_STREAM_ENV_T *pStreamEnv, uint8_t *ptrBuf, uint32_t length)
{
    uint32_t btclk; //hal slot -- 312.5us
    uint16_t btcnt; //hal  microsecond -- 0.5 us
    uint32_t dmaIrqHappeningTimeUs = 0;
    uint8_t adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
    uint32_t len = 0;
    POSSIBLY_UNUSED int32_t diff_bt_time = 0;
    adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    len = length;
    if (adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt, adma_ch&0xFF, dma_base);
        dmaIrqHappeningTimeUs = bt_syn_ble_bt_time_to_bts(btclk, btcnt);
    }
    // it's possible the multiple DMA irq triggered message accumulates,
    // so the acuiqred dmaIrqHappeningTimeUs is the last dma irq, which has
    // been handled. For this case, ignore this dma irq message handling
    if ((!pStreamEnv) ||
        (GAF_PLAYBACK_STREAM_START_TRIGGERING > pStreamEnv->stream_context.playback_stream_state) ||
        ((GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED == pStreamEnv->stream_context.playback_stream_state) &&
        (dmaIrqHappeningTimeUs == pStreamEnv->stream_context.lastPlaybackDmaIrqTimeUs)))
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
        gaf_stream_common_clr_trigger(pStreamEnv->stream_context.playbackTriggerChannel);
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

    /*******************************************stream encode and send*********************************************/
    uint8_t ase_count = gaf_cis_mobile_stream_type_op_rule_all.playback_ase_count;
    uint32_t sample_cnt;
#ifdef __BLE_AUDIO_24BIT__
    sample_cnt = len/sizeof(int32_t);
    int32_t pcm_buf[ase_count][sample_cnt];
#else
    sample_cnt = len/sizeof(int16_t);
    int16_t pcm_buf[ase_count][sample_cnt];
#endif
    memset(pcm_buf, 0, sizeof(pcm_buf));

    uint8_t connected_iso_num = 0;
    uint8_t ase_cnt_per_conn = GAF_AUDIO_ASE_TOTAL_COUNT / BLE_AUDIO_CONNECTION_CNT;
    uint8_t algo_ctx_idx = 0;

    for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
    {
        if (pStreamEnv->stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl == GAF_AUDIO_INVALID_ISO_CHANNEL)
        {
            continue;
        }
        connected_iso_num++;
        // only can handle 2 iso data
        if (connected_iso_num > 2)
        {
            LOG_E("iso connected num is more than 2");
            connected_iso_num = 2;
            break;
        }

        // Calculate this iso data should be handle by which lc3 decode
        algo_ctx_idx = (i / ase_cnt_per_conn);

#ifdef GAF_DECODER_CROSS_CORE_USE_M55
        if (true == is_support_ble_audio_mobile_m55_decode)
        {
            uint16_t lastChannelSeqnumb = 0;
            GAF_DECODER_CC_MEDIA_DATA_T *pRet = NULL;
            GAF_DECODER_CC_MCU_ENV_T *gaf_mobile_decoder_cc_env = gaf_mobile_m55_get_decoder_cc_env();
            pRet = (GAF_DECODER_CC_MEDIA_DATA_T *)GetCQueueReadPointer(&(gaf_mobile_decoder_cc_env->cachedDecodedDataFromCoreQueue));
            if (NULL == pRet)
            {
                break;
            }
            else
            {
                lastChannelSeqnumb = pRet->pkt_seq_nb;
            }   ///pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]

            bool isSuccessful = gaf_decoder_core_fetch_pcm_data(pStreamEnv, lastChannelSeqnumb,
                                            (uint8_t *)pcm_buf[algo_ctx_idx], length, dmaIrqHappeningTimeUs);
            if (!isSuccessful)
            {
                memset(pcm_buf[algo_ctx_idx], 0, length);
            }
        }
#else
        int ret = 0;
        GAF_AUDIO_STREAM_COMMON_INFO_T playback_Info = pStreamEnv->stream_info.playbackInfo;
        // check for channel num
        uint32_t audio_allocation_bf = playback_Info.aseChInfo[i].allocation_bf;
        uint8_t audio_allocation_cnt = bes_ble_audio_get_location_fs_l_r_cnt(audio_allocation_bf);
        gaf_media_data_t * decoder_frame_p_fill = NULL;
        gaf_media_data_t *decoder_frame_p = NULL;
        decoder_frame_p = gaf_mobile_audio_get_packet(pStreamEnv, dmaIrqHappeningTimeUs, playback_Info.aseChInfo[i].iso_channel_hdl);
        LOG_D("%s %d %d %d length %d", __func__, i, decoder_frame_p->data_len, playback_Info.aseChInfo[i].iso_channel_hdl, length);
        /// Check packet is lack of left or right
        bool is_right_channel = ((audio_allocation_bf & (BES_BLE_LOC_SIDE_RIGHT | BES_BLE_LOC_FRONT_RIGHT)) != 0);
        /// calculate max packet len
        uint32_t packet_max_len = pStreamEnv->stream_info.playbackInfo.num_channels * pStreamEnv->stream_info.playbackInfo.encoded_frame_size;
        /// shift to used channel
        uint32_t channel_shift = is_right_channel ? (packet_max_len/2) : 0;
        /// fill up stereo channel pcm
        if (pStreamEnv->stream_info.playbackInfo.num_channels == AUD_CHANNEL_NUM_2 &&
            audio_allocation_cnt == AUD_CHANNEL_NUM_1)
        {
            /// Malloc packet max len to fill lc3 decoder
            decoder_frame_p_fill = (gaf_media_data_t *)gaf_stream_data_frame_malloc(packet_max_len);
            ASSERT(decoder_frame_p_fill, "Invalid Frame NULL %d", __LINE__);
            memset_s(decoder_frame_p_fill->sdu_data, packet_max_len, 0, packet_max_len);
            /// assign params
            decoder_frame_p_fill->pkt_status = decoder_frame_p->pkt_status;
            decoder_frame_p_fill->pkt_seq_nb = decoder_frame_p->pkt_seq_nb;
            decoder_frame_p_fill->time_stamp = decoder_frame_p->time_stamp;
            /// Move channel data, left channel remains zero
            memcpy_s(decoder_frame_p_fill->sdu_data + channel_shift, decoder_frame_p->data_len,
                     decoder_frame_p->sdu_data, decoder_frame_p->data_len);
            /// Free previous channel packet
            gaf_stream_data_free(decoder_frame_p);
            decoder_frame_p = decoder_frame_p_fill;
        }
        if (decoder_frame_p->pkt_status != GAF_ISO_PKT_STATUS_INVALID)
        {
            ret = pStreamEnv->func_list->decoder_func_list->decoder_decode_frame_func
                        (decoder_frame_p->isPLC, decoder_frame_p->data_len, decoder_frame_p->sdu_data,
                            &pStreamEnv->stream_context.codec_alg_context[algo_ctx_idx], pcm_buf[algo_ctx_idx]);
            LOG_D("dec ret %d", ret);

            if (ret)
            {
                memset(pcm_buf[algo_ctx_idx], 0, length);
            }
            // use one iso data to do pid
            if (connected_iso_num == 1)
            {
                diff_bt_time = GAF_AUDIO_CLK_32_BIT_DIFF(decoder_frame_p->time_stamp +
                                                        (GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US),
                                                        dmaIrqHappeningTimeUs);
                gaf_media_pid_adjust(AUD_STREAM_PLAYBACK, &(pStreamEnv->stream_context.playback_pid_env),
                                            diff_bt_time);
                LOG_D("index %d Decoded seq 0x%02x expected play time %u local time %u diff %d",
                    i, decoder_frame_p->pkt_seq_nb, decoder_frame_p->time_stamp, dmaIrqHappeningTimeUs, diff_bt_time);
            }
        }
        /// gaf custom, may be a watch point to add some pattern in pcm data to be encoded
        if (gaf_uc_cli_custom_data_callback->decoded_raw_data_cb)
        {
            gaf_uc_cli_custom_data_callback->decoded_raw_data_cb(decoder_frame_p, ptrBuf, length);
        }
        gaf_stream_data_free(decoder_frame_p);
#endif
    }

#if defined(AUDIO_STREAM_TEST_AUDIO_DUMP)
    audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(0 , pcm_buf[0],sample_cnt, 2, 0, 8);
    audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(1 , pcm_buf[1],sample_cnt, 2, 1, 8);
    audio_dump_run();
#endif

/// TODO: Why use only 16bit for this case
#if !defined (__BLE_AUDIO_24BIT__)  && defined (BLE_AUDIO_USE_TWO_MIC_SRC_FOR_DONGLE)
#define OUTPUT_MIC_NUM 0
#if (OUTPUT_MIC_NUM == 1)
    for (uint32_t samples = 0; samples < length / 2; samples++)
    {
        ((int16_t *)ptrBuf)[samples] = pcm_buf[samples];
    }
#elif (OUTPUT_MIC_NUM == 2)
    for (uint32_t samples = 0; samples < length / 2; samples+=2)
    {
        ((int16_t *)ptrBuf)[samples] = (int16_t)(((int32_t)pcm_buf[samples] + (int32_t)pcm_buf[samples+1])/2);
        ((int16_t *)ptrBuf)[samples+1] = (int16_t)ptrBuf[samples];
        }
#elif (OUTPUT_MIC_NUM == 4)
    for (uint32_t samples = 0; samples < length / 2; samples+=2)
    {
        ((int16_t *)ptrBuf)[samples] = (int16_t)(((int32_t)pcm_buf[samples] + (int32_t)pcm_buf[samples+1] +
                                (int32_t)pcm_buf[samples+length/2] + (int32_t)pcm_buf[samples+length/2+1])/4);
        ((int16_t *)ptrBuf)[samples+1] = (int16_t)ptrBuf[samples];
    }
#endif  // OUTPUT_MIC_NUM
#else   // BLE_AUDIO_USE_TWO_MIC_SRC_FOR_DONGLE == 0 or __BLE_AUDIO_24BIT__
    // Merge stream data from multiple devices iso data
    if (connected_iso_num == 2)
    {
#ifdef __BLE_AUDIO_24BIT__
        for (uint32_t samples = 0; samples < sample_cnt; samples++)
        {
            ((int32_t *)ptrBuf)[samples] =
                (int32_t)(((int32_t)pcm_buf[0][samples] + (int32_t)pcm_buf[1][samples])/2);
        }
#else
        for (uint32_t samples = 0; samples < sample_cnt; samples++)
        {
            ((int16_t *)ptrBuf)[samples] =
                (int16_t)(((int32_t)pcm_buf[0][samples] + (int32_t)pcm_buf[1][samples])/2);
        }
#endif
    }
    /// TODO: is index 0 the real device index, may be 1 is the real index
    else if (connected_iso_num == 1)
    {
#ifdef __BLE_AUDIO_24BIT__
        for (uint32_t samples = 0; samples < sample_cnt; samples++)
        {
            ((int32_t *)ptrBuf)[samples] = (int32_t)pcm_buf[0][samples];
        }
#else
        for (uint32_t samples = 0; samples < sample_cnt; samples++)
        {
            ((int16_t *)ptrBuf)[samples] = (int16_t)pcm_buf[0][samples];
        }
#endif
    }
    else// if (connected_iso_num == 0 || connected_iso_num > 2)
    {
       memset(ptrBuf, 0, length);
    }
#endif  // BLE_AUDIO_USE_TWO_MIC_SRC_FOR_DONGLE
}

POSSIBLY_UNUSED static int gaf_mobile_audio_flexible_playback_stream_start_handler(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    LOG_I("%s start", __func__);
    if (GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state)
    {
        // TODO: shall use reasonable cpu frequency
        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_208M);
        af_set_priority(AF_USER_AI, osPriorityHigh);
        struct AF_STREAM_CONFIG_T stream_cfg;
        memset((void *)&stream_cfg, 0, sizeof(struct AF_STREAM_CONFIG_T));
        stream_cfg.bits         = (enum AUD_BITS_T)(pStreamEnv->stream_info.playbackInfo.bits_depth);
        stream_cfg.channel_num  = (enum AUD_CHANNEL_NUM_T)(pStreamEnv->stream_info.playbackInfo.num_channels);
        stream_cfg.io_path      = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.device       = AUD_STREAM_USE_INT_CODEC;
        if (stream_cfg.channel_num == AUD_CHANNEL_NUM_2)
        {
           stream_cfg.channel_map  = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1);
        }
        stream_cfg.sample_rate  = (enum AUD_SAMPRATE_T)pStreamEnv->stream_info.playbackInfo.sample_rate;
        // TODO: get vol from VCC via ase_lid
        stream_cfg.vol          = TGT_VOLUME_LEVEL_7;
        stream_cfg.data_size    = (uint32_t)(2 * pStreamEnv->stream_info.playbackInfo.dmaChunkSize);
        pStreamEnv->func_list->stream_func_list.playback_init_stream_buf_func(pStreamEnv);
        pStreamEnv->func_list->decoder_func_list->decoder_init_func(pStreamEnv,
            gaf_cis_mobile_stream_type_op_rule_all.playback_ase_count);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(pStreamEnv->stream_info.playbackInfo.dmaBufPtr);
        stream_cfg.handler = pStreamEnv->func_list->stream_func_list.playback_dma_irq_handler_func;
        bes_ble_bap_dp_itf_data_come_callback_register((void *)gaf_mobile_audio_receive_data);
        af_codec_tune(AUD_STREAM_PLAYBACK, 0);
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
#if defined(AUDIO_STREAM_TEST_AUDIO_DUMP)
        audio_dump_init(480, sizeof(short), 2);
#endif
        pStreamEnv->stream_context.playbackTriggerChannel = app_bt_sync_get_available_trigger_channel(APP_BT_SYNC_OP_MAX, APP_BT_SYNC_POLICY_DEFAULT);
        gaf_media_prepare_playback_trigger(pStreamEnv->stream_context.playbackTriggerChannel);
        // put PID env into stream context
        gaf_media_pid_init(&(pStreamEnv->stream_context.playback_pid_env));
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        gaf_stream_common_update_playback_stream_state(pStreamEnv, GAF_PLAYBACK_STREAM_INITIALIZED);
        return 0;
    }
    return -1;
}
POSSIBLY_UNUSED static void gaf_mobile_audio_flexible_common_buf_init(GAF_AUDIO_STREAM_ENV_T * pStreamEnv, GAF_STREAM_TYPE_E stream_type)
{
    if ((GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state) &&
        (GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state))
    {
        pStreamEnv->stream_info.playbackInfo.storedDmaBufPtr = NULL;
        pStreamEnv->stream_info.captureInfo.storedDmaBufPtr = NULL;
#ifndef AOB_CODEC_CP
        lc3_alloc_data_free();
#endif
        app_audio_mempool_init_with_specific_size(app_audio_mempool_size());
        uint32_t audioCacheHeapSize = 0;
        uint8_t* heapBufStartAddr = NULL;
        if (GAF_STREAM_PLAYBACK == stream_type)
        {
            audioCacheHeapSize = 2*(pStreamEnv->stream_info.playbackInfo.maxCachedEncodedAudioPacketCount*
                pStreamEnv->stream_info.playbackInfo.maxEncodedAudioPacketSize *
                gaf_cis_mobile_stream_type_op_rule_all.playback_ase_count);
        }
        else
        {
            audioCacheHeapSize = 2*(pStreamEnv->stream_info.captureInfo.maxCachedEncodedAudioPacketCount*
                pStreamEnv->stream_info.captureInfo.maxEncodedAudioPacketSize);
        }
        app_audio_mempool_get_buff(&heapBufStartAddr, audioCacheHeapSize);
        gaf_stream_heap_init(heapBufStartAddr, audioCacheHeapSize);
    }
    if (GAF_STREAM_PLAYBACK == stream_type)
    {
        if (NULL == pStreamEnv->stream_info.playbackInfo.storedDmaBufPtr)
        {
            app_audio_mempool_get_buff(&(pStreamEnv->stream_info.playbackInfo.storedDmaBufPtr),
                            pStreamEnv->stream_info.playbackInfo.dmaChunkSize*2);
        }
        pStreamEnv->stream_info.playbackInfo.dmaBufPtr = pStreamEnv->stream_info.playbackInfo.storedDmaBufPtr;
    }
    else
    {
        if (NULL == pStreamEnv->stream_info.captureInfo.storedDmaBufPtr)
        {
            app_audio_mempool_get_buff(&(pStreamEnv->stream_info.captureInfo.storedDmaBufPtr),
                            pStreamEnv->stream_info.captureInfo.dmaChunkSize*2);
        }
        pStreamEnv->stream_info.captureInfo.dmaBufPtr = pStreamEnv->stream_info.captureInfo.storedDmaBufPtr;
    }
}

static void gaf_mobile_audio_flexible_playback_buf_init(void* _pStreamEnv)
{
    LOG_D("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    gaf_mobile_audio_flexible_common_buf_init(pStreamEnv, GAF_STREAM_PLAYBACK);
    for (uint8_t i = 0; i < gaf_cis_mobile_stream_type_op_rule_all.playback_ase_count; i++)
    {
        gaf_list_new(&pStreamEnv->stream_context.playback_buff_list[i].buff_list,
                        (osMutex(gaf_mobile_decoder_buffer_mutex)),
                        gaf_stream_data_free,
                        gaf_stream_heap_cmalloc,
                        gaf_stream_heap_free);
    }
    pStreamEnv->func_list->decoder_func_list->decoder_init_buf_func(pStreamEnv,
        gaf_cis_mobile_stream_type_op_rule_all.playback_ase_count);
    LOG_D("%s end", __func__);
}
POSSIBLY_UNUSED static int gaf_mobile_audio_flexible_playback_stream_stop_handler(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
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
    app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_32K);
    af_set_priority(AF_USER_AI, osPriorityAboveNormal);
    pStreamEnv->func_list->decoder_func_list->decoder_deinit_func();
    pStreamEnv->func_list->stream_func_list.playback_deinit_stream_buf_func(pStreamEnv);
    return 0;
}
static uint32_t gaf_mobile_stream_flexible_playback_dma_irq_handler(uint8_t* ptrBuf, uint32_t length)
{
    LOG_D("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_mobile_audio_stream_env;
    gaf_mobile_audio_process_encoded_data(pStreamEnv, ptrBuf, length);
    LOG_D("%s end", __func__);
    return length;
}
static void gaf_mobile_audio_flexible_playback_buf_deinit(void* _pStreamEnv)
{
    LOG_I("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    pStreamEnv->stream_info.playbackInfo.dmaBufPtr = NULL;
    pStreamEnv->func_list->decoder_func_list->decoder_deinit_buf_func(pStreamEnv,
        gaf_cis_mobile_stream_type_op_rule_all.playback_ase_count);

    LOG_I("%s end", __func__);
}

POSSIBLY_UNUSED static int gaf_mobile_audio_flexible_capture_stream_start_handler(void* _pStreamEnv)
{
    LOG_D("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    if (GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state)
    {
        // TODO: shall use reasonable cpu frequency
#ifdef AOB_UC_TEST
        app_sysfreq_req(APP_SYSFREQ_USER_AOB, (enum APP_SYSFREQ_FREQ_T)mobile_freq);
#else
        app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_208M);
#endif
        af_set_priority(AF_USER_AI, osPriorityHigh);
        struct AF_STREAM_CONFIG_T stream_cfg;
        // capture stream
        memset((void *)&stream_cfg, 0, sizeof(struct AF_STREAM_CONFIG_T));
        stream_cfg.bits         = (enum AUD_BITS_T)(pStreamEnv->stream_info.captureInfo.bits_depth);
        stream_cfg.channel_num  = (enum AUD_CHANNEL_NUM_T)(pStreamEnv->stream_info.captureInfo.num_channels);
        stream_cfg.channel_map  = (enum AUD_CHANNEL_MAP_T)(AUD_CHANNEL_MAP_CH0|AUD_CHANNEL_MAP_CH1);
 #ifdef WIRELESS_MIC
        stream_cfg.io_path      = AUD_INPUT_PATH_MAINMIC;
 #else
        stream_cfg.io_path      = AUD_INPUT_PATH_LINEIN;
 #endif
        stream_cfg.device       = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.sample_rate  = (enum AUD_SAMPRATE_T)pStreamEnv->stream_info.captureInfo.sample_rate;
        // TODO: get vol from VCC via ase_lid
        stream_cfg.vol          = TGT_ADC_VOL_LEVEL_7;
        stream_cfg.data_size    = (uint32_t)(2 * pStreamEnv->stream_info.captureInfo.dmaChunkSize);
        pStreamEnv->func_list->stream_func_list.capture_init_stream_buf_func(pStreamEnv);
        pStreamEnv->func_list->encoder_func_list->encoder_init_func(pStreamEnv);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(pStreamEnv->stream_info.captureInfo.dmaBufPtr);
        stream_cfg.handler = pStreamEnv->func_list->stream_func_list.capture_dma_irq_handler_func;
        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
        pStreamEnv->stream_context.captureTriggerChannel = app_bt_sync_get_available_trigger_channel(APP_BT_SYNC_OP_MAX, APP_BT_SYNC_POLICY_DEFAULT);
        gaf_media_prepare_capture_trigger(pStreamEnv->stream_context.captureTriggerChannel);
        gaf_media_pid_init(&(pStreamEnv->stream_context.capture_pid_env));
        gaf_media_pid_update_threshold(&(pStreamEnv->stream_context.capture_pid_env),
            pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs/2);
        gaf_stream_common_update_capture_stream_state(pStreamEnv, GAF_CAPTURE_STREAM_INITIALIZED);
        if (GAF_AUDIO_TRIGGER_BY_ISOLATE_STREAM  ==
            gaf_cis_mobile_stream_type_op_rule_all.trigger_stream_type)
        {
            uint32_t latest_iso_bt_time = gaf_media_common_get_latest_tx_iso_evt_timestamp(pStreamEnv);
            uint32_t current_bt_time = gaf_media_sync_get_curr_time();
            uint32_t trigger_bt_time = latest_iso_bt_time +
                (pStreamEnv->stream_info.captureInfo.cigSyncDelayUs/pStreamEnv->stream_info.captureInfo.bnM2S/2);
            // move ahead of trigger time by 1ms to leave more margin for long CIG delay
            if (pStreamEnv->stream_info.captureInfo.cigSyncDelayUs > 3000)
            {
                trigger_bt_time -= 1000;
            }

            while (trigger_bt_time < current_bt_time + GAF_AUDIO_MAX_DIFF_BT_TIME)
            {
                trigger_bt_time += pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs;
            }

            LOG_I("iso anch %d cur time %d trigger time %d",
                latest_iso_bt_time, current_bt_time, trigger_bt_time);
            gaf_stream_common_set_capture_trigger_time(pStreamEnv, trigger_bt_time);
        }
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        return 0;
    }
    return -1;
}

static void gaf_mobile_audio_flexible_capture_buf_init(void* _pStreamEnv)
{
    LOG_I("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    gaf_mobile_audio_flexible_common_buf_init(pStreamEnv, GAF_STREAM_CAPTURE);
#ifdef GAF_CODEC_CROSS_CORE
    gaf_list_new(&pStreamEnv->stream_context.m55_capture_buff_list.buff_list,
                    (osMutex(gaf_m55_encoder_buffer_mutex)),
                    gaf_m55_stream_encoder_data_free,
                    gaf_stream_heap_cmalloc,
                    gaf_m55_stream_encoder_heap_free);
#else
    gaf_list_new(&pStreamEnv->stream_context.capture_buff_list,
                    (osMutex(gaf_mobile_encoder_buffer_mutex)),
                    gaf_stream_data_free,
                    gaf_stream_heap_cmalloc,
                    gaf_stream_heap_free);
#endif
    pStreamEnv->func_list->encoder_func_list->encoder_init_buf_func(pStreamEnv);
    LOG_I("%s end", __func__);
}
POSSIBLY_UNUSED static int gaf_mobile_audio_flexible_capture_stream_stop_handler(void* _pStreamEnv)
{
    LOG_I("%s stop", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    uint8_t POSSIBLY_UNUSED adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
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
    gaf_stream_common_clr_trigger(pStreamEnv->stream_context.captureTriggerChannel);
    app_sysfreq_req(APP_SYSFREQ_USER_AOB, APP_SYSFREQ_32K);
    af_set_priority(AF_USER_AI, osPriorityAboveNormal);
    pStreamEnv->func_list->encoder_func_list->encoder_deinit_func(pStreamEnv);
    pStreamEnv->func_list->stream_func_list.capture_deinit_stream_buf_func(pStreamEnv);
    return 0;
}

static uint32_t gaf_mobile_stream_flexible_capture_dma_irq_handler(uint8_t* ptrBuf, uint32_t length)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_mobile_audio_stream_env;
    gaf_mobile_audio_process_pcm_data(pStreamEnv, ptrBuf, length);
    return length;
}
static void gaf_mobile_audio_flexible_capture_buf_deinit(void* _pStreamEnv)
{
    LOG_I("%s start", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    pStreamEnv->stream_info.captureInfo.dmaBufPtr = NULL;
#ifdef GAF_CODEC_CROSS_CORE
    gaf_list_free(&pStreamEnv->stream_context.m55_capture_buff_list.buff_list);
#else
    gaf_list_free(&pStreamEnv->stream_context.capture_buff_list);
#endif
    pStreamEnv->func_list->encoder_func_list->encoder_deinit_buf_func(pStreamEnv);
    LOG_I("%s end", __func__);
}
static GAF_AUDIO_FUNC_LIST_T gaf_mobile_audio_flexible_stream_func_list =
{
    {
        .playback_dma_irq_handler_func = gaf_mobile_stream_flexible_playback_dma_irq_handler,
        .capture_dma_irq_handler_func = gaf_mobile_stream_flexible_capture_dma_irq_handler,
#ifdef BLE_USB_AUDIO_SUPPORT
        .playback_start_stream_func = gaf_mobile_usb_audio_capture_start_handler,
        .playback_init_stream_buf_func = gaf_mobile_audio_flexible_playback_buf_init,
        .playback_stop_stream_func = gaf_mobile_usb_audio_capture_stop_handler,
#else
        .playback_start_stream_func = gaf_mobile_audio_flexible_playback_stream_start_handler,
        .playback_init_stream_buf_func = gaf_mobile_audio_flexible_playback_buf_init,
        .playback_stop_stream_func = gaf_mobile_audio_flexible_playback_stream_stop_handler,
#endif
        .playback_deinit_stream_buf_func = gaf_mobile_audio_flexible_playback_buf_deinit,
#ifdef BLE_USB_AUDIO_SUPPORT
        .capture_start_stream_func = gaf_mobile_usb_audio_media_stream_start_handler,
        .capture_init_stream_buf_func = gaf_mobile_audio_flexible_capture_buf_init,
        .capture_stop_stream_func = gaf_mobile_usb_audio_media_stream_stop_handler,
#else
        .capture_start_stream_func = gaf_mobile_audio_flexible_capture_stream_start_handler,
        .capture_init_stream_buf_func = gaf_mobile_audio_flexible_capture_buf_init,
        .capture_stop_stream_func = gaf_mobile_audio_flexible_capture_stream_stop_handler,
#endif
        .capture_deinit_stream_buf_func = gaf_mobile_audio_flexible_capture_buf_deinit,
    },
};
static GAF_AUDIO_STREAM_ENV_T* gaf_mobile_audio_get_stream_env_from_ase(uint8_t ase_lid)
{
    return &gaf_mobile_audio_stream_env;
}

static GAF_AUDIO_STREAM_ENV_T* gaf_mobile_audio_refresh_stream_info_from_ase(uint8_t ase_lid, uint8_t idx)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = gaf_mobile_audio_get_stream_env_from_ase(ase_lid);
    const bes_ble_bap_ascc_ase_t *p_ase_info = bes_ble_bap_ascc_get_ase_info(ase_lid);

    GAF_AUDIO_STREAM_COMMON_INFO_T* pCommonInfo;

    if (p_ase_info == NULL)
    {
        LOG_W("%s ASE info is NULL!!!", __func__);
        return pStreamEnv;
    }

    /*
    * Direction is defined from the perspective of the ASE server.
    * So for mobile phone, sink is capture stream.
    */
    if (BES_BLE_GAF_DIRECTION_SINK == p_ase_info->direction)
    {
        pCommonInfo = &(pStreamEnv->stream_info.captureInfo);
        /// should always use stereo channel for mobile capture
        pCommonInfo->num_channels = GAF_MOBILE_AUDIO_STREAM_CAPTURE_CHANNEL_NUM;
    }
    else
    {
        pCommonInfo = &(pStreamEnv->stream_info.playbackInfo);
        /// depend on uc srv capablity
        pCommonInfo->num_channels =
        bes_ble_bap_pacc_is_peer_support_stereo_channel(p_ase_info->con_lid, BES_BLE_GAF_DIRECTION_SRC) ?
                                    AUD_CHANNEL_NUM_2 : AUD_CHANNEL_NUM_1;
        if (GAF_MOBILE_AUDIO_STREAM_PLAYBACK_CHANNEL_NUM < pCommonInfo->num_channels)
        {
            LOG_E("%s unsupport channel num %d", __func__, pCommonInfo->num_channels);
        }

        pStreamEnv->stream_context.playback_buff_list[idx].cisChannel =
                                                            BLE_ISOHDL_TO_ACTID(p_ase_info->cis_hdl);
        LOG_I("%s cisChannel %d conlid %d", __func__,
            pStreamEnv->stream_context.playback_buff_list[idx].cisChannel, p_ase_info->con_lid);
    }
    // cig-cis timing info
    pCommonInfo->cigSyncDelayUs = p_ase_info->cig_sync_delay;
    pCommonInfo->isoIntervalUs = p_ase_info->iso_interval_us;
    pCommonInfo->bnM2S = p_ase_info->bn_m2s;
    pCommonInfo->bnS2M = p_ase_info->bn_s2m;
    // codec info
    pCommonInfo->bits_depth = GAF_MOBILE_AUDIO_STREAM_BIT_NUM;
    // Check codec cfg
    if (p_ase_info->p_cfg == NULL)
    {
        LOG_E("codec cfg null");
        return pStreamEnv;
    }
    pCommonInfo->aseChInfo[p_ase_info->ase_lid].allocation_bf = p_ase_info->p_cfg->param.location_bf;
    LOG_I("%s direction:%d codec_id:%d con_lid %d ase_id:%d", __func__,
        p_ase_info->direction, p_ase_info->codec_id.codec_id[0], p_ase_info->con_lid, ase_lid);

    LOG_I("cig sync delay %d us - iso interval %d us - bnM2S %d - bnS2M %d",
        pCommonInfo->cigSyncDelayUs, pCommonInfo->isoIntervalUs, pCommonInfo->bnM2S,
        pCommonInfo->bnS2M);
    switch (p_ase_info->codec_id.codec_id[0])
    {
        case AOB_CODEC_TYPE_LC3:
        {
            AOB_BAP_CFG_T* p_lc3_cfg = p_ase_info->p_cfg;
            pCommonInfo->frame_ms =
                gaf_stream_common_frame_duration_parse(p_lc3_cfg->param.frame_dur);
            pCommonInfo->sample_rate =
                gaf_stream_common_sample_freq_parse(p_lc3_cfg->param.sampling_freq);
            pCommonInfo->encoded_frame_size = p_lc3_cfg->param.frame_octet;
            pCommonInfo->maxCachedEncodedAudioPacketCount = GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER;
            pCommonInfo->maxEncodedAudioPacketSize = gaf_audio_lc3_encoder_get_max_frame_size();
            pCommonInfo->dmaChunkIntervalUs = (uint32_t)(pCommonInfo->frame_ms*1000);
#ifdef __BLE_AUDIO_24BIT__
            //24Bits force align to 32bits
            pCommonInfo->dmaChunkSize =
                (uint32_t)((pCommonInfo->sample_rate*
                (32/8)*
                (pCommonInfo->dmaChunkIntervalUs)*
                pCommonInfo->num_channels)/(1000*1000));
#else
            pCommonInfo->dmaChunkSize =
                (uint32_t)((pCommonInfo->sample_rate*
                (pCommonInfo->bits_depth/8)*
                (pCommonInfo->dmaChunkIntervalUs)*
                pCommonInfo->num_channels)/(1000*1000));
#endif
            gaf_audio_lc3_update_codec_func_list(pStreamEnv);
            break;
        }
#ifdef LC3PLUS_SUPPORT
        case AOB_CODEC_TYPE_VENDOR:
        {
            AOB_BAP_CFG_T* p_lc3plus_cfg = p_ase_info->p_cfg;
            pCommonInfo->frame_ms =
                gaf_stream_common_frame_duration_parse(p_lc3plus_cfg->param.frame_dur);
            pCommonInfo->sample_rate =
                gaf_stream_common_sample_freq_parse(p_lc3plus_cfg->param.sampling_freq);
            pCommonInfo->maxCachedEncodedAudioPacketCount = GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER;
            pCommonInfo->maxEncodedAudioPacketSize = gaf_audio_lc3_encoder_get_max_frame_size();
            pCommonInfo->encoded_frame_size = p_lc3plus_cfg->param.frame_octet;
            if (2.5 == pCommonInfo->frame_ms)
            {
                pCommonInfo->maxCachedEncodedAudioPacketCount = GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER*2;
                pCommonInfo->maxEncodedAudioPacketSize = gaf_audio_lc3_encoder_get_max_frame_size()/2;
            }
            pCommonInfo->dmaChunkIntervalUs = (uint32_t)(pCommonInfo->frame_ms*1000);
            pCommonInfo->bits_depth = AUD_BITS_24;
            //24Bits force align to 32bits
            pCommonInfo->dmaChunkSize =
                (uint32_t)(((pCommonInfo->sample_rate/1000)*
                (32/8)*
                (pCommonInfo->dmaChunkIntervalUs)*
                pCommonInfo->num_channels)/1000);
            gaf_audio_lc3plus_update_codec_func_list(pStreamEnv);
            break;
        }
#endif
        default:
            ASSERT(false, "unknown codec type!");
            return NULL;
    }
    pCommonInfo->aseChInfo[p_ase_info->ase_lid].iso_channel_hdl = BLE_ISOHDL_TO_ACTID(p_ase_info->cis_hdl);
    pCommonInfo->aseChInfo[p_ase_info->ase_lid].cis_hdl = p_ase_info->cis_hdl;
    LOG_I("frmae encode size = %d",pCommonInfo->encoded_frame_size);
    LOG_I("cis handle 0x%x ,iso_channel_hdl %d", p_ase_info->cis_hdl,
        pCommonInfo->aseChInfo[p_ase_info->ase_lid].iso_channel_hdl);
    LOG_I("frame len %d us, sample rate %d dma chunk time %d us dma chunk size %d",
        (uint32_t)(pCommonInfo->frame_ms*1000), pCommonInfo->sample_rate,
        pCommonInfo->dmaChunkIntervalUs, pCommonInfo->dmaChunkSize);
    LOG_I("allocation bf: 0x%x", pCommonInfo->aseChInfo[p_ase_info->ase_lid].allocation_bf);
    LOG_I("codec: %s", gaf_stream_common_print_code_type(p_ase_info->codec_id.codec_id[0]));
    LOG_I("context: %s", gaf_stream_common_print_context(p_ase_info->p_metadata->param.context_bf));
    return pStreamEnv;
}

static uint8_t gaf_mobile_audio_get_enabled_playback_ase_count(void)
{
    uint8_t enabled_ase_count = 0;
    for (uint8_t con_id = 0;con_id < GAF_MOB_MAXIMUM_CONNECTION_COUNT;con_id++)
    {
        for (uint8_t idx = 0; idx < GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT; idx++)
        {
           enabled_ase_count +=
            (gaf_cis_mobile_media_dwelling_info[con_id].playback_ase_id[idx] !=
                GAF_INVALID_ASE_INDEX);
        }

    }
    return enabled_ase_count;
}

static uint8_t gaf_mobile_audio_get_enabled_capture_ase_count(void)
{
    uint8_t enabled_ase_count = 0;
    for (uint8_t con_id = 0;con_id < GAF_MOB_MAXIMUM_CONNECTION_COUNT;con_id++)
    {
        for (uint8_t idx = 0; idx < GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT; idx++)
        {
           enabled_ase_count +=
            (gaf_cis_mobile_media_dwelling_info[con_id].capture_ase_id[idx] !=
                GAF_INVALID_ASE_INDEX);
        }
    }
    return enabled_ase_count;
}
static gaf_stream_context_state_t gaf_mobile_audio_update_stream_info_from_ase(
                                GAF_AUDIO_UPDATE_STREAM_INFO_PURPOSE_E purpose, uint8_t ase_lid, uint8_t con_lid)
{
    uint8_t enabled_playback_ase_cnt = 0;
    uint8_t enabled_capture_ase_cnt = 0;
    uint8_t gaf_connected_dev_num = 0;
    const bes_ble_bap_ascc_ase_t *p_bap_ase_info = bes_ble_bap_ascc_get_ase_info(ase_lid);
    GAF_MEDIA_DWELLING_INFO_T* pDewellingInfo = &gaf_cis_mobile_media_dwelling_info[con_lid];
    /*
    * Direction is defined from the perspective of the ASE server.
    * So for mobile phone, sink is capture stream.
    */
    LOG_I("update stream ase %d purpose %d", ase_lid, purpose);

    if (BES_BLE_GAF_DIRECTION_SINK == p_bap_ase_info->direction)
    {
        if (GAF_AUDIO_UPDATE_STREAM_INFO_TO_START == purpose)
        {
            pDewellingInfo->startedStreamTypes |= GAF_AUDIO_STREAM_TYPE_CAPTURE;
        }
        else if (GAF_AUDIO_UPDATE_STREAM_INFO_TO_STOP == purpose)
        {
            pDewellingInfo->startedStreamTypes &= (~GAF_AUDIO_STREAM_TYPE_CAPTURE);
        }
    }
    else
    {
        if (GAF_AUDIO_UPDATE_STREAM_INFO_TO_START == purpose)
        {
            pDewellingInfo->startedStreamTypes |= GAF_AUDIO_STREAM_TYPE_PLAYBACK;
        }
        else if (GAF_AUDIO_UPDATE_STREAM_INFO_TO_STOP == purpose)
        {
            pDewellingInfo->startedStreamTypes &= (~GAF_AUDIO_STREAM_TYPE_PLAYBACK);
        }
    }

    LOG_I("direction %d started stream types updated to %d", p_bap_ase_info->direction,
        pDewellingInfo->startedStreamTypes);

    if (GAF_AUDIO_UPDATE_STREAM_INFO_TO_START == purpose)
    {
        if (BES_BLE_GAF_DIRECTION_SINK == p_bap_ase_info->direction)
        {
            gaf_mobile_audio_add_ase_into_capture_ase_list(p_bap_ase_info->con_lid, ase_lid);
        }
        else
        {
            gaf_mobile_audio_add_ase_into_playback_ase_list(p_bap_ase_info->con_lid, ase_lid);
        }

        enabled_playback_ase_cnt = gaf_mobile_audio_get_enabled_playback_ase_count();
        enabled_capture_ase_cnt = gaf_mobile_audio_get_enabled_capture_ase_count();
        gaf_connected_dev_num = gaf_mobile_get_connected_device_count();

        if (GAF_AUDIO_STREAM_TYPE_FLEXIBLE ==
            gaf_cis_mobile_stream_type_op_rule_all.included_stream_type)
        {
            // any direction of stream can start the context
            return APP_GAF_CONTEXT_STREAM_STARTED;
        }
        else
        {
            if (((pDewellingInfo->startedStreamTypes &
                gaf_cis_mobile_stream_type_op_rule_all.included_stream_type) ==
                gaf_cis_mobile_stream_type_op_rule_all.included_stream_type) &&
                (enabled_playback_ase_cnt >= gaf_connected_dev_num) &&
                (enabled_capture_ase_cnt >= gaf_connected_dev_num))
            {
                return APP_GAF_CONTEXT_STREAM_STARTED;
            }
            else
            {
                LOG_I("Still miss a ase to start, need playback ase cnt %d capture ase cnt %d",
                    gaf_connected_dev_num,
                    gaf_connected_dev_num);
                LOG_I("Currently enabled playback ase cnt %d capture ase cnt %d",
                    enabled_playback_ase_cnt, enabled_capture_ase_cnt);
                return APP_GAF_CONTEXT_SINGLE_STREAM_STOPPED;
            }
        }
    }
    else
    {
        if (BES_BLE_GAF_DIRECTION_SINK == p_bap_ase_info->direction)
        {
            gaf_mobile_audio_remove_ase_from_capture_ase_list(p_bap_ase_info->con_lid, ase_lid);
        }
        else
        {
            gaf_mobile_audio_remove_ase_from_playback_ase_list(p_bap_ase_info->con_lid, ase_lid);
        }

        enabled_playback_ase_cnt = gaf_mobile_audio_get_enabled_playback_ase_count();
        enabled_capture_ase_cnt = gaf_mobile_audio_get_enabled_capture_ase_count();
        gaf_connected_dev_num = gaf_mobile_get_connected_device_count();

        if ((0 == enabled_playback_ase_cnt)
            && (0 == enabled_capture_ase_cnt)
            && (0 == pDewellingInfo->startedStreamTypes))
        {
            return APP_GAF_CONTEXT_ALL_STREAMS_STOPPED;
        }
        else
        {
            if (GAF_AUDIO_STREAM_TYPE_FLEXIBLE !=
                gaf_cis_mobile_stream_type_op_rule_all.included_stream_type)
            {
                LOG_I("Still miss a stream type to stop.");
                LOG_I("Currently enabled playback ase cnt %d capture ase cnt %d",
                    enabled_playback_ase_cnt, enabled_capture_ase_cnt);
            }
            return APP_GAF_CONTEXT_SINGLE_STREAM_STOPPED;
        }
    }

    LOG_I("enabled playback/capture ase cnt: %d/%d, gaf connected dev cnt: %d",
            enabled_playback_ase_cnt, enabled_capture_ase_cnt, gaf_connected_dev_num);
}

static GAF_AUDIO_STREAM_ENV_T* _gaf_get_mobile_stream_info(void)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = NULL;
    GAF_AUDIO_STREAM_ENV_T* pPlaybackStreamEnv = NULL;
    GAF_AUDIO_STREAM_ENV_T* pCaptureStreamEnv = NULL;
    uint8_t idx = 0;
    for (uint8_t con_id = 0; con_id < GAF_MOB_MAXIMUM_CONNECTION_COUNT; con_id++)
    {
        uint8_t *playback_ase_lid_list = gaf_mobile_audio_get_playback_ase_index_list(con_id);
        uint8_t *capture_ase_lid_list = gaf_mobile_audio_get_capture_ase_index_list(con_id);

        for (idx = 0; idx < GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT; idx++)
        {
            if (GAF_INVALID_ASE_INDEX != capture_ase_lid_list[idx])
            {
                pStreamEnv = gaf_mobile_audio_refresh_stream_info_from_ase(capture_ase_lid_list[idx], idx);
                pPlaybackStreamEnv = (pStreamEnv == NULL) ?
                                            pPlaybackStreamEnv : pStreamEnv;
                // No break for update all un refresh stream info
            }
        }

        for (idx = 0; idx < GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT; idx++)
        {
            if (GAF_INVALID_ASE_INDEX != playback_ase_lid_list[idx])
            {
                pStreamEnv = gaf_mobile_audio_refresh_stream_info_from_ase(playback_ase_lid_list[idx], idx);
                pPlaybackStreamEnv = (pStreamEnv == NULL) ?
                                            pPlaybackStreamEnv : pStreamEnv;
                // No break for update all un refresh stream info
            }
        }

        if (pPlaybackStreamEnv && pCaptureStreamEnv)
        {
            ASSERT(pPlaybackStreamEnv == pCaptureStreamEnv, "Same context should use the same env!");
        }

        if (NULL == pStreamEnv)
        {
            if (pPlaybackStreamEnv)
            {
                pStreamEnv = pPlaybackStreamEnv;
            }
            else if (pCaptureStreamEnv)
            {
                pStreamEnv = pCaptureStreamEnv;
            }
        }
    }

    return pStreamEnv;
}

static bool gaf_mobile_audio_is_all_used_ase_streaming(uint8_t direction)
{
    uint8_t capture_ase_used_cnt = gaf_mobile_audio_get_enabled_capture_ase_count();

    uint8_t nb_ase = 0;
    uint8_t ase_lid_list[GAF_AUDIO_ASE_TOTAL_COUNT] = {0};
    for (uint8_t i = 0; i < AOB_COMMON_MOBILE_CONNECTION_MAX; i++)
    {
        nb_ase = bes_ble_bap_ascc_get_specific_state_ase_lid_list(
                            i, direction, BES_BLE_GAF_ASCS_ASE_STATE_ENABLING, ase_lid_list);
        if (nb_ase != 0)
        {
            LOG_I("Wait all ase enter streaming to send iso together, %d/%d",
                                                        nb_ase, capture_ase_used_cnt);
            return false;
        }
    }

    return true;
}

static void _gaf_mobile_audio_stream_start(uint8_t con_lid)
{
#ifdef GAF_DSP
    dsp_open();
#endif
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = _gaf_get_mobile_stream_info();
    GAF_MEDIA_DWELLING_INFO_T* pDewellingInfo = &gaf_cis_mobile_media_dwelling_info[con_lid];

    bool all_ase_streaming_sink = gaf_mobile_audio_is_all_used_ase_streaming((uint8_t)BES_BLE_GAF_DIRECTION_SINK);
    bool all_ase_streaming_src = gaf_mobile_audio_is_all_used_ase_streaming((uint8_t)BES_BLE_GAF_DIRECTION_SRC);

    if (pStreamEnv)
    {
        LOG_I("%s stream type = %d, both connected %d streaming %d/%d state %d/%d", __func__,
                pDewellingInfo->startedStreamTypes,
                ble_audio_mobile_both_is_connected(),
                all_ase_streaming_sink,
                all_ase_streaming_src,
                pStreamEnv->stream_context.capture_stream_state,
                pStreamEnv->stream_context.playback_stream_state);

        if ((pDewellingInfo->startedStreamTypes&GAF_AUDIO_STREAM_TYPE_PLAYBACK) &&
            (GAF_PLAYBACK_STREAM_IDLE == pStreamEnv->stream_context.playback_stream_state))
        {
            if (all_ase_streaming_src)
            {
                pStreamEnv->func_list->stream_func_list.playback_start_stream_func(pStreamEnv);
            }
        }

        if ((pDewellingInfo->startedStreamTypes&GAF_AUDIO_STREAM_TYPE_CAPTURE) &&
            (GAF_CAPTURE_STREAM_IDLE == pStreamEnv->stream_context.capture_stream_state))
        {
            if (all_ase_streaming_sink)
            {
                pStreamEnv->func_list->stream_func_list.capture_start_stream_func(pStreamEnv);
            }
        }
    }
}

// TODO: to be registered to media manager module
static void gaf_mobile_audio_stream_start(uint8_t con_lid)
{
    bt_adapter_write_sleep_enable(0); // Current thread is BT thread
    app_bt_start_custom_function_in_bt_thread((uint32_t)con_lid,
                                              0,
                                              (uint32_t)_gaf_mobile_audio_stream_start);
}

static void _gaf_mobile_audio_stream_start_handler(uint8_t ase_lid, uint8_t con_lid)
{
    gaf_stream_context_state_t updatedContextStreamState =
        gaf_mobile_audio_update_stream_info_from_ase(GAF_AUDIO_UPDATE_STREAM_INFO_TO_START, ase_lid, con_lid);
    if (APP_GAF_CONTEXT_STREAM_STARTED == updatedContextStreamState)
    {
        gaf_mobile_audio_stream_start(con_lid);
    }
}

void gaf_mobile_audio_stream_update_and_start_handler(uint8_t ase_lid, uint8_t con_lid)
{
    LOG_I("%s ase_lid = %d, con_lid = %d", __func__, ase_lid, con_lid);
    app_bt_start_custom_function_in_bt_thread(
        (uint32_t)ase_lid,(uint32_t)con_lid,(uint32_t)_gaf_mobile_audio_stream_start_handler);
}

static void _gaf_mobile_audio_stream_stop(uint8_t con_lid)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_mobile_audio_stream_env;

    uint8_t ase_cnt_per_conn = GAF_AUDIO_ASE_TOTAL_COUNT / BLE_AUDIO_CONNECTION_CNT;
    if (pStreamEnv)
    {
        GAF_MEDIA_DWELLING_INFO_T* pDewellingInfo_L = &gaf_cis_mobile_media_dwelling_info[0];
        GAF_MEDIA_DWELLING_INFO_T* pDewellingInfo_R = &gaf_cis_mobile_media_dwelling_info[1];
        GAF_MEDIA_DWELLING_INFO_T* pDewellingInfo = &gaf_cis_mobile_media_dwelling_info[con_lid];

        LOG_D("%s, [con %d]startedStreamTypes %d",__func__, con_lid, pDewellingInfo->startedStreamTypes);
        LOG_D("%s, [con %d]startedStreamTypes_L %d,startedStreamTypes_R %d",
            __func__, con_lid, pDewellingInfo_L->startedStreamTypes,pDewellingInfo_R->startedStreamTypes);

        // Only both buds stop stream, dongle stop stream
        if ((0 == (pDewellingInfo_L->startedStreamTypes&GAF_AUDIO_STREAM_TYPE_PLAYBACK)) &&
            (0 == (pDewellingInfo_R->startedStreamTypes&GAF_AUDIO_STREAM_TYPE_PLAYBACK)) &&
            (GAF_PLAYBACK_STREAM_IDLE != pStreamEnv->stream_context.playback_stream_state))
        {
            pStreamEnv->stream_context.playback_stream_state = GAF_PLAYBACK_STREAM_IDLE;
            pStreamEnv->func_list->stream_func_list.playback_stop_stream_func(pStreamEnv);
        }

        if ((0 == (pDewellingInfo_L->startedStreamTypes & GAF_AUDIO_STREAM_TYPE_CAPTURE)) &&
#if !defined (BLE_USB_AUDIO_SUPPORT) || !defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
            (0 == (pDewellingInfo_R->startedStreamTypes & GAF_AUDIO_STREAM_TYPE_CAPTURE)) &&
#endif
            (GAF_CAPTURE_STREAM_IDLE != pStreamEnv->stream_context.capture_stream_state))
        {
            pStreamEnv->stream_context.capture_stream_state = GAF_CAPTURE_STREAM_IDLE;
            pStreamEnv->func_list->stream_func_list.capture_stop_stream_func(pStreamEnv);
        }

        if (0 == (pDewellingInfo->startedStreamTypes&GAF_AUDIO_STREAM_TYPE_CAPTURE))
        {
            // only stop tx hdl that belong to specific connection
            for (uint8_t i = ase_cnt_per_conn*con_lid; i < ase_cnt_per_conn*(con_lid + 1); i++)
            {
                if (pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl == GAF_AUDIO_INVALID_ISO_CHANNEL)
                {
                    continue;
                }
                bes_ble_bap_dp_tx_iso_stop(pStreamEnv->stream_info.captureInfo.aseChInfo[i].cis_hdl);
                pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
            }
        }

        if (0 == (pDewellingInfo->startedStreamTypes&GAF_AUDIO_STREAM_TYPE_PLAYBACK))
        {
            // only clear rx hdl that belong to specific connection
            for (uint8_t i = ase_cnt_per_conn*con_lid; i < ase_cnt_per_conn*(con_lid + 1); i++)
            {
                pStreamEnv->stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
            }
        }

        if (pStreamEnv->stream_context.capture_stream_state == GAF_CAPTURE_STREAM_IDLE &&
            pStreamEnv->stream_context.playback_stream_state == GAF_PLAYBACK_STREAM_IDLE)
        {
            bt_adapter_write_sleep_enable(1); // Current thread is BT thread
        }
    }
#ifdef GAF_DSP
    dsp_close();
#endif
}

// TODO: to be registered to media manager module
static void gaf_mobile_audio_stream_stop(uint8_t con_lid)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)con_lid,
                                              0,
                                              (uint32_t)_gaf_mobile_audio_stream_stop);
}

#ifdef GAF_CODEC_CROSS_CORE
/**
 ****************************************************************************************
 * @brief mobile bth core send deinit signal to  m55 core with rsp
 *
 * @param[in] con_id                       connection index
 *
 * @param[out] NONE                      NONE
 ****************************************************************************************
 */
static void gaf_mobile_audio_stream_bth_send_deinit_to_m55(uint8_t ase_lid)
{
    /// Get ase info through ase_lid
    uint8_t con_id = 0;
    const bes_ble_bap_ascc_ase_t *p_bap_ase_info = bes_ble_bap_ascc_get_ase_info(ase_lid);
    con_id = p_bap_ase_info->con_lid;
    /// get con_id, context_type, is_bis, is_bis_src value
    gaf_m55_deinit_status.con_lid        = con_id;
    gaf_m55_deinit_status.context_type   = 0;
    gaf_m55_deinit_status.is_bis         = false;
    gaf_m55_deinit_status.is_bis_src     = false;
    gaf_m55_deinit_status.is_mobile_role = true;
    if (p_bap_ase_info->direction == BES_BLE_GAF_DIRECTION_SINK)
    {
        gaf_m55_deinit_status.capture_deinit = true;
    }
    else if (p_bap_ase_info->direction == BES_BLE_GAF_DIRECTION_SRC)
    {
        gaf_m55_deinit_status.playback_deinit = true;
    }

    GAF_AUDIO_M55_DEINIT_T p_mobile_deinit_req;
    p_mobile_deinit_req.con_lid         = gaf_m55_deinit_status.con_lid;
    p_mobile_deinit_req.context_type    = gaf_m55_deinit_status.context_type;
    p_mobile_deinit_req.capture_deinit  = gaf_m55_deinit_status.capture_deinit;
    p_mobile_deinit_req.playback_deinit = gaf_m55_deinit_status.playback_deinit;
    p_mobile_deinit_req.is_bis          = gaf_m55_deinit_status.is_bis;
    p_mobile_deinit_req.is_bis_src      = gaf_m55_deinit_status.is_bis_src;
    p_mobile_deinit_req.is_mobile_role  = gaf_m55_deinit_status.is_mobile_role;

    if ((gaf_m55_deinit_status.capture_deinit_sent == false)
            && (gaf_m55_deinit_status.capture_deinit == true)){
        app_dsp_m55_bridge_send_cmd(
                CROSS_CORE_TASK_CMD_GAF_ENCODE_DEINIT_WAITING_RSP,
                (uint8_t*)&p_mobile_deinit_req,
                sizeof(GAF_AUDIO_M55_DEINIT_T));
        gaf_m55_deinit_status.capture_deinit_sent = true;
    }

    if (gaf_m55_deinit_status.playback_deinit_sent == false
            && (gaf_m55_deinit_status.playback_deinit == true)){
        app_dsp_m55_bridge_send_cmd(
                CROSS_CORE_TASK_CMD_GAF_DECODE_DEINIT_WAITING_RSP,
                (uint8_t*)&p_mobile_deinit_req,
                sizeof(GAF_AUDIO_M55_DEINIT_T));
        gaf_m55_deinit_status.playback_deinit_sent = true;
    }
}

/**
 ****************************************************************************************
 * @brief When mobile bth received the encoder deinit signal from m55, bth will deinit m55 core
 *
 * @param[in] con_id                       connection index
 *
 * @param[out] NONE                      NONE
 ****************************************************************************************
 */
void _gaf_mobile_audio_bth_encoder_received_deinit_signal_from_m55(uint8_t con_id)
{
    LOG_D("gaf_mobile_audio_bth_received_deinit_signal_from_m55");
    app_dsp_m55_deinit(APP_DSP_M55_USER_AUDIO_ENCODER);
}
/**
 ****************************************************************************************
 * @brief When mobile bth received the decoder deinit signal from m55, bth will deinit m55 core
 *
 * @param[in] con_id                       connection index
 *
 * @param[out] NONE                      NONE
 ****************************************************************************************
 */
void _gaf_mobile_audio_bth_decoder_received_deinit_signal_from_m55(uint8_t con_id)
{
    LOG_D("gaf_mobile_audio_bth_received_deinit_signal_from_m55");
    app_dsp_m55_deinit(APP_DSP_M55_USER_AUDIO_DECODER);
}
#endif

static void _gaf_mobile_audio_stream_stop_handler(uint8_t ase_lid, uint8_t con_lid)
{
    gaf_stream_context_state_t updatedContextStreamState =
        gaf_mobile_audio_update_stream_info_from_ase(GAF_AUDIO_UPDATE_STREAM_INFO_TO_STOP, ase_lid, con_lid);

    if (updatedContextStreamState == APP_GAF_CONTEXT_ALL_STREAMS_STOPPED
        || updatedContextStreamState == APP_GAF_CONTEXT_SINGLE_STREAM_STOPPED)
    {
        gaf_mobile_audio_stream_stop(con_lid);
#ifdef GAF_CODEC_CROSS_CORE
        gaf_mobile_audio_stream_bth_send_deinit_to_m55(ase_lid);
#endif
    }
}
void gaf_mobile_audio_stream_update_and_stop_handler(uint8_t ase_lid, uint8_t con_lid)
{
    LOG_I("%s, con_lid=%d", __func__, con_lid);
    app_bt_start_custom_function_in_bt_thread((uint32_t)ase_lid,
                                              (uint32_t)con_lid,
                                              (uint32_t)_gaf_mobile_audio_stream_stop_handler);
}

static void gaf_mobile_audio_steam_retrigger_handler(void* stream_env)
{
    GAF_MEDIA_DWELLING_INFO_T gaf_cis_mobile_media_dwelling_info_rec[GAF_MOB_MAXIMUM_CONNECTION_COUNT];
    // Record
    memcpy_s(gaf_cis_mobile_media_dwelling_info_rec,
                sizeof(gaf_cis_mobile_media_dwelling_info),
                gaf_cis_mobile_media_dwelling_info,
                sizeof(gaf_cis_mobile_media_dwelling_info));
    // Clear
    memset_s(gaf_cis_mobile_media_dwelling_info,
                sizeof(GAF_MEDIA_DWELLING_INFO_T),
                0,
                sizeof(GAF_MEDIA_DWELLING_INFO_T));
    // Stop
    for (uint8_t con_lid = 0; con_lid < GAF_MOB_MAXIMUM_CONNECTION_COUNT; con_lid++)
    {
        _gaf_mobile_audio_stream_stop(con_lid);
    }
    // Recovery
    memcpy_s(gaf_cis_mobile_media_dwelling_info,
                sizeof(gaf_cis_mobile_media_dwelling_info),
                gaf_cis_mobile_media_dwelling_info_rec,
                sizeof(gaf_cis_mobile_media_dwelling_info));
    // Start
    for (uint8_t con_lid = 0; con_lid < GAF_MOB_MAXIMUM_CONNECTION_COUNT; con_lid++)
    {
        gaf_mobile_audio_stream_start(con_lid);
    }
}

void gaf_mobile_audio_stream_init(void)
{
    ///Get custom callback function ptr
    gaf_uc_cli_custom_data_callback = gaf_stream_common_get_custom_data_handler(GAF_STREAM_USER_CASE_UC_CLI);
    /// Sanity check
    ASSERT(gaf_uc_cli_custom_data_callback, "Invalid custom data callback, user case %d", GAF_STREAM_USER_CASE_UC_CLI);

    memset(gaf_cis_mobile_media_dwelling_info, 0, sizeof(gaf_cis_mobile_media_dwelling_info));

    uint8_t con_id = 0;
    for (con_id = 0; con_id < GAF_MOB_MAXIMUM_CONNECTION_COUNT; con_id++)
    {
        gaf_mobile_audio_clear_playback_ase_index_list(con_id);
        gaf_mobile_audio_clear_capture_ase_index_list(con_id);
    }
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = &gaf_mobile_audio_stream_env;
    memset((uint8_t *)pStreamEnv, 0, sizeof(GAF_AUDIO_STREAM_ENV_T));
    pStreamEnv->stream_context.playback_stream_state = GAF_PLAYBACK_STREAM_IDLE;
    pStreamEnv->stream_context.capture_stream_state = GAF_CAPTURE_STREAM_IDLE;
    pStreamEnv->stream_context.playback_trigger_supervisor_timer_id = NULL;
    pStreamEnv->stream_context.capture_trigger_supervisor_timer_id = NULL;
    pStreamEnv->stream_info.is_mobile = true;
    for (uint8_t i = 0; i < GAF_AUDIO_ASE_TOTAL_COUNT; i++)
    {
        pStreamEnv->stream_info.playbackInfo.aseChInfo[i].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
        pStreamEnv->stream_info.captureInfo.aseChInfo[i].iso_channel_hdl = GAF_AUDIO_INVALID_ISO_CHANNEL;
    }
    GAF_AUDIO_STREAM_INFO_T* pStreamInfo;
    pStreamInfo = &(gaf_mobile_audio_stream_env.stream_info);
    pStreamInfo->contextType = GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE;
    gaf_stream_common_register_func_list(&gaf_mobile_audio_stream_env,
        &gaf_mobile_audio_flexible_stream_func_list);
    gaf_mobile_stream_register_retrigger_callback(gaf_mobile_audio_steam_retrigger_handler);
}
#endif
