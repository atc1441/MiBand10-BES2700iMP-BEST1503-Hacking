/**
 *
 * @copyright Copyright (c) 2015-2022 BES Technic.
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
#include "bluetooth_ble_api.h"
#include "hal_trace.h"
#include "hal_timer.h"
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
#include "hal_location.h"
#include "lc3_process.h"

#include "gaf_stream_dbg.h"
#include "gaf_media_pid.h"
#include "gaf_media_stream.h"
#include "gaf_codec_cc_bth.h"


#include "smf_api.h"
//#include "ble_audio_earphone_info.h"
#ifdef GAF_CODEC_CROSS_CORE
#include "gaf_codec_cc_common.h"
#include "gaf_media_common.h"
#include "app_dsp_m55.h"
#include "mcu_dsp_m55_app.h"
#include "rwble_config.h"
#endif

#ifdef GAF_CODEC_CROSS_CORE
/************************private macro defination***************************/

/***** encoder section ******/
#define GAF_ENCODER_MAX_CACHED_ENCODED_FRAME_CNT       (25)
#define GAF_MAXIMUM_WAITING_ENCODED_DATA_CNT           (3)
#define GAF_WAITING_ENCODED_DATA_MS_PER_ROUND          (2)
#define GAF_ENCODER_MAX_LEN_IN_MAILBOX                 (512)

/***** decoder section ******/
#define GAF_DECODER_MAX_CACHED_ENCODED_FRAME_CNT       (25)
#define GAF_MAXIMUM_WAITING_DECODED_DATA_CNT           (4)
#define GAF_WAITING_DECODED_DATA_MS_PER_ROUND          (2)
#define GAF_DECODER_MAX_LEN_IN_MAILBOX                 (512)
#define TIME_CALCULATE_REVERSAL_THRESHOLD              0x7F000000

#ifdef AOB_MOBILE_ENABLED
#define GAF_MOBILE_AUDIO_MAX_DIFF_BT_TIME              (4000)
#endif

/************************private strcuture defination****************************/

/***** encoder section ******/
/***** decoder section ******/

/************************private variable defination************************/

/***** encoder section ******/
static uint8_t gaf_encoder_max_cached_encoded_frame_ptr_queue_buf[GAF_ENCODER_MAX_CACHED_ENCODED_FRAME_CNT * sizeof(GAF_ENCODER_CC_MEDIA_DATA_T)];
#ifdef AOB_MOBILE_ENABLED
static uint8_t encoded_cnt = 0;
#endif
/***** decoder section ******/
static uint8_t gaf_decoder_max_cached_encoded_frame_ptr_queue_buf[GAF_DECODER_MAX_CACHED_ENCODED_FRAME_CNT * sizeof(GAF_DECODER_CC_MEDIA_DATA_T)];


/************************global variable defination************************/

/***** encoder section ******/
GAF_ENCODER_CC_MCU_ENV_T gaf_encoder_cc_mcu_env;
GAF_AUDIO_M55_DEINIT_T gaf_bth_encoder_recived_deinit_req;

/***** decoder section ******/
GAF_DECODER_CC_MCU_ENV_T gaf_decoder_cc_mcu_env;
GAF_AUDIO_M55_DEINIT_T gaf_bth_decoder_recived_deinit_req;
GAF_AUDIO_STREAM_ENV_T* pLocalCaptureStreamEnvPtr;
GAF_AUDIO_STREAM_ENV_T* pLocalCaptureBisStreamEnvPtr;

#ifdef AOB_MOBILE_ENABLED
GAF_AUDIO_STREAM_ENV_T* pLocalMobileCaptureStreamEnvPtr;
#endif


/************************extern variable defination************************/

/***** encoder section ******/

/***** decoder section ******/
extern GAF_AUDIO_STREAM_ENV_T* pLocalStreamEnvPtr;
extern GAF_AUDIO_STREAM_ENV_T* pLocalBisStreamEnvPtr;
#ifdef AOB_MOBILE_ENABLED
extern GAF_AUDIO_STREAM_ENV_T* pLocalMobileStreamEnvPtr;
#endif


/**********************private function declaration*************************/

/***** encoder section ******/
static void gaf_encoder_mcu_deinit_handler(void);
static GAF_ENCODER_CC_MEDIA_DATA_T* gaf_encoder_core_peek_from_cached_encoded(void);
static void gaf_encoder_mcu_init_req_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_mcu_ack_feed_encoded_data_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_mcu_ack_feed_encoded_data_success_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_mcu_init_wait_rsp_timeout_handler(uint8_t*, uint16_t);
static void gaf_encoder_mcu_init_rsp_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_mcu_deinit_req_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_mcu_deinit_wait_rsp_timeout_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_mcu_retrigger_req_received_handler(uint8_t* param, uint16_t len);
static void gaf_encoder_mcu_feed_encoded_data_received_handler(uint8_t* param, uint16_t len);
static void gaf_encoder_mcu_ack_feed_encoded_data_to_m55(GAF_ENCODER_CC_MEDIA_DATA_T* pMediaPacket);
static void gaf_audio_encoder_core_pop_oldest_cached_encoded_data(void* _pStreamEnv);
static void gaf_encoder_mcu_deinit_rsp_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_mcu_feed_pcm_data_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_mcu_ack_fed_encoded_data_transmit_handler(uint8_t* ptr, uint16_t len);

/***** decoder section ******/
static GAF_DECODER_CC_MEDIA_DATA_T* gaf_decoder_core_peek_from_cached_pcm(void);
static void gaf_decoder_mcu_init_req_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_mcu_deinit_req_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_mcu_deinit_wait_rsp_timeout_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_mcu_deinit_rsp_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_mcu_feed_encoded_data_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_mcu_ack_fed_encoded_data_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_mcu_retrigger_req_received_handler(uint8_t* param, uint16_t len);
static void gaf_decoder_mcu_fed_pcm_data_received_handler(uint8_t* param, uint16_t len);
static void gaf_decoder_mcu_rm_specific_frame_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_mcu_ack_fed_pcm_data_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_cc_init_rsp_received_handler(uint8_t* ptr, uint16_t len);

/****************************function defination****************************/

/***** encoder section ******/
RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ENCODE_INIT_WAITING_RSP,
                            "CC_GAF_INIT_WAITING_RSP",
                            gaf_encoder_mcu_init_req_transmit_handler,
                            NULL,
                            5000,
                            gaf_encoder_mcu_init_wait_rsp_timeout_handler,
                            gaf_encoder_mcu_init_rsp_received_handler,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ENCODE_DEINIT_WAITING_RSP,
                            "CC_GAF_ENCODE_DEINIT_WAITING_RSP",
                            gaf_encoder_mcu_deinit_req_transmit_handler,
                            NULL,
                            1000,
                            gaf_encoder_mcu_deinit_wait_rsp_timeout_handler,
                            gaf_encoder_mcu_deinit_rsp_received_handler,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ACK_ENCODE_FEED_PCM_DATA_PACKET,
                            "ACK_FEED_PCM_DATA_PACKET",
                            NULL,
                            gaf_encoder_mcu_ack_feed_encoded_data_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ACK_FEED_PCM_DATA_SUCCEED,
                            "ACK_FEED_PCM_DATA_PACKET_SUCCESSED",
                            NULL,
                            gaf_encoder_mcu_ack_feed_encoded_data_success_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_RETRIGGER_REQ_NO_RSP,
                            "CC_GAF_RETRIIGER_REQ_NO_RSP",
                            NULL,
                            gaf_encoder_mcu_retrigger_req_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_FEED_ENCODED_DATA_TO_CORE,
                            "FEED_ENCODED_DATA_TO_CORE",
                            NULL,
                            gaf_encoder_mcu_feed_encoded_data_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ENCODE_FEED_PCM_DATA_PACKET,
                            "CC_FEED_PCM_DATA_PACKET",
                            gaf_encoder_mcu_feed_pcm_data_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ACK_FEED_ENCODED_DATA_TO_CORE,
                            "CC_ACK_FED_ENCODED_DATA_PACKET",
                            gaf_encoder_mcu_ack_fed_encoded_data_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

/***** decoder section ******/
M55_CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(CROSS_CORE_INSTANT_CMD_GAF_DECODE_INIT_WAITING_RSP,
                            gaf_decoder_mcu_init_req_transmit_handler,
                            NULL);

M55_CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(CROSS_CORE_INSTANT_CMD_GAF_DECODE_INIT_RSP_TO_CORE,
                            NULL,
                            gaf_decoder_cc_init_rsp_received_handler);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_DEINIT_WAITING_RSP,
                            "CC_GAF_DECODE_DEINIT_WAITING_RSP",
                            gaf_decoder_mcu_deinit_req_transmit_handler,
                            NULL,
                            1000,
                            gaf_decoder_mcu_deinit_wait_rsp_timeout_handler,
                            gaf_decoder_mcu_deinit_rsp_received_handler,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_ENCODED_DATA_PACKET,
                            "CC_FEED_ENCODED_DATA_PACKET",
                            gaf_decoder_mcu_feed_encoded_data_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_ACK_FEED_ENCODED_DATA_PACKET,
                            "ACK_FED_ENCODED_DATA_PACKET",
                            NULL,
                            gaf_decoder_mcu_ack_fed_encoded_data_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_RETRIGGER_REQ_NO_RSP,
                            "CC_GAF_RETRIGGER_REQ_NO_RSP",
                            NULL,
                            gaf_decoder_mcu_retrigger_req_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_PCM_DATA_PACKET,
                            "CC_FEED_PCM_DATA_PACKET",
                            NULL,
                            gaf_decoder_mcu_fed_pcm_data_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_ACK_FED_PCM_DATA_PACKET,
                            "CC_ACK_FED_PCM_DATA_PACKET",
                            gaf_decoder_mcu_ack_fed_pcm_data_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_REMOVE_SPECIFIC_FRAME,
                            "GAF_REMOVE_SPECIFIC_FRAME",
                            gaf_decoder_mcu_rm_specific_frame_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

/***** encoder section ******/
static void gaf_encoder_mcu_accumulate_cmd_send(uint16_t cmd_code, uint8_t *data)
{
    uint8_t fetchedData[GAF_ENCODER_MAX_LEN_IN_MAILBOX];
    uint16_t lenFetched = 0;
    uint8_t maxAllowCnt = 0;
    uint16_t unitOfInfo = 0;
    uint16_t threshold = GAF_ENCODER_MAX_LEN_IN_MAILBOX;
    switch (cmd_code)
    {
        case CROSS_CORE_TASK_CMD_GAF_ENCODE_FEED_PCM_DATA_PACKET:
            unitOfInfo = sizeof(GAF_ENCODER_CC_MEDIA_DATA_T);
        break;
        default:
            ASSERT(0, "%s, Wrong cmdCode:0x%x", __func__, cmd_code);
        break;
    }

    // Accumulate when there are too many mailbox cnts
    //  @APP_DSP_M55_BRIDGE_TX_MAILBOX_MAX
    if (app_dsp_m55_bridge_get_tx_mailbox_cnt() < 5)
    {
        goto send_directly;
    }

    maxAllowCnt = (GAF_ENCODER_MAX_LEN_IN_MAILBOX / unitOfInfo);
    threshold = (maxAllowCnt - 1) * unitOfInfo;

    app_dsp_m55_bridge_fetch_tx_mailbox(cmd_code, fetchedData, &lenFetched, threshold);
send_directly:
    memcpy(fetchedData + lenFetched, data, unitOfInfo);
    app_dsp_m55_bridge_send_cmd(cmd_code, fetchedData, lenFetched + unitOfInfo);
}

static void gaf_decoder_cc_init_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    if (pLocalStreamEnvPtr)
    {
        if (GAF_PLAYBACK_STREAM_WAITING_M55_INIT == pLocalStreamEnvPtr->stream_context.playback_stream_state)
        {
            gaf_stream_common_update_playback_stream_state(pLocalStreamEnvPtr, GAF_PLAYBACK_STREAM_INITIALIZED);
            gaf_decoder_cc_mcu_env.offCoreIsReady = true;
        }
    }
    else if (pLocalBisStreamEnvPtr)
    {
        if (GAF_PLAYBACK_STREAM_WAITING_M55_INIT == pLocalBisStreamEnvPtr->stream_context.playback_stream_state)
        {
            gaf_stream_common_update_playback_stream_state(pLocalBisStreamEnvPtr, GAF_PLAYBACK_STREAM_INITIALIZED);
            gaf_decoder_cc_mcu_env.offCoreIsReady = true;
        }
    }
}

static void gaf_encoder_mcu_ack_fed_encoded_data_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_ACK_FEED_ENCODED_DATA_TO_CORE, ptr, len);
}

static void gaf_encoder_mcu_feed_pcm_data_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_ENCODE_FEED_PCM_DATA_PACKET, ptr, len);
}

static void gaf_encoder_mcu_ack_feed_encoded_data_received_handler(uint8_t* ptr, uint16_t len)
{
    GAF_ENCODER_ACK_FEED_PCM_DATA_REQ_T* pReq = (GAF_ENCODER_ACK_FEED_PCM_DATA_REQ_T*)ptr;
    // update the decoded and pcm list entry count on the off core side
    gaf_encoder_cc_mcu_env.encodedPacketEntryCount = pReq->encodedPacketEntryCount;
    gaf_encoder_cc_mcu_env.pcmPacketEntryCount     = pReq->pcmPacketEntryCount;
    gaf_encoder_cc_mcu_env.cachedPacketCount       = gaf_encoder_cc_mcu_env.encodedPacketEntryCount + gaf_encoder_cc_mcu_env.pcmPacketEntryCount;
}

static void gaf_encoder_mcu_rm_m55_received_pcm_packet(GAF_AUDIO_STREAM_ENV_T* pLocalStreamEnvPtr)
{
    gaf_stream_buff_list_t* list_info = NULL;
    list_info = &(pLocalStreamEnvPtr->stream_context.m55_capture_buff_list.buff_list);
    if (!list_info) {
        LOG_W("%s, list null", __func__);
        return;
    }
    gaf_list_remove_generic(list_info, gaf_encoder_cc_mcu_env.coorespondingpcmDataPointer);
}

static void gaf_encoder_mcu_ack_feed_encoded_data_success_received_handler(uint8_t* ptr, uint16_t len)
{
    GAF_ENCODER_ACK_FEED_PCM_DATA_SUCC_REQ_T* pReq     = (GAF_ENCODER_ACK_FEED_PCM_DATA_SUCC_REQ_T*) ptr;
    gaf_encoder_cc_mcu_env.is_m55_received_pcm_data    = pReq->is_successed;
    gaf_encoder_cc_mcu_env.coorespondingpcmDataPointer = pReq->coorespondingPcmDataPacketPtr;

    /// if m55 received pcm data, it will delete the pcm data packet from capture buff list
    if (gaf_encoder_cc_mcu_env.is_m55_received_pcm_data){
        gaf_encoder_cc_mcu_env.is_m55_received_pcm_data = false;
        if (NULL != pLocalCaptureStreamEnvPtr){
            if ((GAF_CAPTURE_STREAM_STREAMING_TRIGGERED != pLocalCaptureStreamEnvPtr->stream_context.capture_stream_state) ||
                (false == is_capture_state))
            {
                LOG_I("m55 capture deint or capture stream is not TRIGGERED state!");
                return ;
            }
            gaf_encoder_mcu_rm_m55_received_pcm_packet(pLocalCaptureStreamEnvPtr);
        }

        if (NULL != pLocalCaptureBisStreamEnvPtr){
            if (GAF_CAPTURE_STREAM_STREAMING_TRIGGERED != pLocalCaptureBisStreamEnvPtr->stream_context.capture_stream_state)
            {
                LOG_I("BIS m55 capture deint or capture stream is not TRIGGERED state!");
                return ;
            }
            gaf_encoder_mcu_rm_m55_received_pcm_packet(pLocalCaptureBisStreamEnvPtr);

        }

#ifdef AOB_MOBILE_ENABLED
        if (NULL != pLocalMobileCaptureStreamEnvPtr){
            gaf_encoder_mcu_rm_m55_received_pcm_packet(pLocalMobileCaptureStreamEnvPtr);
        }
#endif
    }
}

static void gaf_encoder_mcu_init_req_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_with_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_ENCODE_INIT_WAITING_RSP, ptr, len);
}

static void gaf_encoder_mcu_init_wait_rsp_timeout_handler(uint8_t*, uint16_t)
{
    ASSERT(false, "wait for gaf init rsp from off mcu core time out!");
}

static void gaf_encoder_mcu_init_rsp_received_handler(uint8_t* ptr, uint16_t len)
{

}

static void gaf_encoder_mcu_deinit_req_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_with_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_ENCODE_DEINIT_WAITING_RSP, ptr, len);
}

static void gaf_encoder_mcu_deinit_wait_rsp_timeout_handler(uint8_t* ptr, uint16_t len)
{
    LOG_I("BTH get gaf_encoder_deinit_rsp from m55 timeout");
    memcpy((uint8_t *)&(gaf_bth_encoder_recived_deinit_req), ptr, sizeof(GAF_AUDIO_M55_DEINIT_T));
    gaf_encoder_mcu_deinit_handler();
}

static void gaf_encoder_mcu_deinit_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    /// bth received m55_deinit_rsp,and to close m55
    LOG_I("BTH gets gaf_encoder_deinit_rsp from m55");
    memcpy((uint8_t *)&(gaf_bth_encoder_recived_deinit_req), ptr, sizeof(GAF_AUDIO_M55_DEINIT_T));
    gaf_encoder_mcu_deinit_handler();
}

static void gaf_encoder_mcu_retrigger_req_received_handler(uint8_t* param, uint16_t len)
{
    // To do: gaf audio media retrigger api
}

static void gaf_encoder_mcu_feed_encoded_data_received_handler(uint8_t* param, uint16_t len)
{
    uint16_t unitofInfo  = sizeof(GAF_ENCODER_CC_MEDIA_DATA_T);
    uint8_t packetCnt = len / unitofInfo;

    while(packetCnt--)
    {
        GAF_ENCODER_CC_MEDIA_DATA_T* p_src_encoded_media_data_info = (GAF_ENCODER_CC_MEDIA_DATA_T *)param;
        uint32_t lock = int_lock();
        int ret = EnCQueue(&gaf_encoder_cc_mcu_env.cachedEncodedDataFromCoreQueue,
            (uint8_t *)p_src_encoded_media_data_info, sizeof(GAF_ENCODER_CC_MEDIA_DATA_T));
        int_unlock(lock);

        if (CQ_OK != ret)
        {
            LOG_E("local mcu audio encoded cache overflow: %u",
                LengthOfCQueue(&gaf_encoder_cc_mcu_env.cachedEncodedDataFromCoreQueue)/
                sizeof(GAF_ENCODER_CC_MEDIA_DATA_T));
            // To do:force gaf audio trigger
        }

        param += unitofInfo;
    }
}

static void gaf_encoder_mcu_ack_feed_encoded_data_to_m55(GAF_ENCODER_CC_MEDIA_DATA_T* pMediaPacket)
{
    app_dsp_m55_bridge_send_cmd(
                        CROSS_CORE_TASK_CMD_GAF_ACK_FEED_ENCODED_DATA_TO_CORE, \
                        (uint8_t*)pMediaPacket, \
                        sizeof(GAF_ENCODER_CC_MEDIA_DATA_T));
}

static void gaf_audio_encoder_core_pop_oldest_cached_encoded_data(void* _pStreamEnv)
{
    GAF_ENCODER_CC_MEDIA_DATA_T* pPopedMediaPacket = NULL;
    uint32_t lock = int_lock();
    if (LengthOfCQueue(&gaf_encoder_cc_mcu_env.cachedEncodedDataFromCoreQueue))
    {
        pPopedMediaPacket = (GAF_ENCODER_CC_MEDIA_DATA_T *)GetCQueueReadPointer(&gaf_encoder_cc_mcu_env.cachedEncodedDataFromCoreQueue);
        DeCQueue(&gaf_encoder_cc_mcu_env.cachedEncodedDataFromCoreQueue, NULL,
            sizeof(GAF_ENCODER_CC_MEDIA_DATA_T));
    }
    int_unlock(lock);

    if (pPopedMediaPacket)
    {
        // inform m55 encoded data is fetched
        gaf_encoder_mcu_ack_feed_encoded_data_to_m55(pPopedMediaPacket);
    }
}

static GAF_ENCODER_CC_MEDIA_DATA_T* gaf_encoder_core_peek_from_cached_encoded(void)
{
    GAF_ENCODER_CC_MEDIA_DATA_T* pRet = NULL;
    uint32_t lock = int_lock();
    if (LengthOfCQueue(&gaf_encoder_cc_mcu_env.cachedEncodedDataFromCoreQueue))
    {
        pRet = (GAF_ENCODER_CC_MEDIA_DATA_T *)GetCQueueReadPointer(&gaf_encoder_cc_mcu_env.cachedEncodedDataFromCoreQueue);
    }
    int_unlock(lock);
    return pRet;
}

bool gaf_encoder_core_fetch_encoded_data(void* _pStreamEnv,
    uint8_t* ptrData, uint16_t dataLen, uint32_t currentDmaIrqTimeUs)
{
    uint8_t waitencodedDataCount = 0;
    GAF_ENCODER_CC_MEDIA_DATA_T *pMediaData = NULL;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    do
    {
    peek_a_packet:
        if (true == gaf_bth_encoder_recived_deinit_req.capture_deinit)
        {
            return false;
        }

        pMediaData = gaf_encoder_core_peek_from_cached_encoded();

        if (NULL == pMediaData)
        {
            LOG_W("cached encoded data queue is empty, wait %d ms until now!",
                waitencodedDataCount*GAF_WAITING_ENCODED_DATA_MS_PER_ROUND);
            waitencodedDataCount++;
            if (GAF_MAXIMUM_WAITING_ENCODED_DATA_CNT >= waitencodedDataCount)
            {
                osDelay(GAF_WAITING_ENCODED_DATA_MS_PER_ROUND);
                goto peek_a_packet;
            }
            else
            {
                return false;
            }
        }

        if (gaf_encoder_cc_mcu_env.isEncodingFirstRun)
        {
            LOG_I("core cross encoding is first run!");
            gaf_encoder_cc_mcu_env.isEncodingFirstRun = false;
        }

        uint32_t expectedDmaIrqHappeningTimeUs =
            pStreamEnv->stream_context.usGapBetweenCapturedFrameAndTransmittedFrame -
            pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs +
            pMediaData->time_stamp;
        int32_t gapUs = GAF_AUDIO_CLK_32_BIT_DIFF(currentDmaIrqTimeUs, expectedDmaIrqHappeningTimeUs);
        // BES intentional code. gapUs will not overflow.

        int32_t gapUs_abs = GAF_AUDIO_ABS(gapUs);
        LOG_D("cur dma ts %d - pkt ts: %d - exp %d - gap %d",
            currentDmaIrqTimeUs, pMediaData->time_stamp, expectedDmaIrqHappeningTimeUs, gapUs_abs);

        if (gapUs_abs < (int32_t)pStreamEnv->stream_info.captureInfo.dmaChunkIntervalUs/2)
        {
            memcpy(ptrData, pMediaData->data, dataLen);
            gaf_audio_encoder_core_pop_oldest_cached_encoded_data(pStreamEnv);
        }
        else
        {
            if (gapUs > 0)
            {
                // not arriving yet, do nothing
                LOG_I("encoded packet's transmission time is not arriving yet.");
            }
            else
            {
                // expired, just pop it and move to the next one
                LOG_I("encoded packet is expired, discard it.");
                gaf_audio_encoder_core_pop_oldest_cached_encoded_data(pStreamEnv);
                goto peek_a_packet;
            }
        }

    }while(0);
    return true;
}

void gaf_encoder_core_reset(void)
{
    uint32_t lock = int_lock();
    gaf_encoder_cc_mcu_env.isGafReceivingJustStarted = true;
    gaf_encoder_cc_mcu_env.isEncodingFirstRun = true;
    gaf_encoder_cc_mcu_env.lastReceivedSeqNum = 0;

    gaf_encoder_cc_mcu_env.encodedPacketEntryCount = 0;
    gaf_encoder_cc_mcu_env.pcmPacketEntryCount = 0;
    gaf_encoder_cc_mcu_env.cachedPacketCount = 0;
    gaf_encoder_cc_mcu_env.is_m55_received_pcm_data = false;
    gaf_encoder_cc_mcu_env.coorespondingpcmDataPointer = NULL;
    gaf_m55_deinit_status.capture_deinit  = false;
    gaf_m55_deinit_status.capture_deinit_sent  = false;
    gaf_m55_deinit_status.is_bis_src      = false;
    gaf_m55_deinit_status.is_mobile_role  = false;
    gaf_bth_encoder_recived_deinit_req.capture_deinit = false;

    InitCQueue(&gaf_encoder_cc_mcu_env.cachedEncodedDataFromCoreQueue,
        sizeof(gaf_encoder_max_cached_encoded_frame_ptr_queue_buf),
        (CQItemType *)gaf_encoder_max_cached_encoded_frame_ptr_queue_buf);

    int_unlock(lock);
}

bool gaf_encoder_mcu_feed_pcm_data_into_off_m55_core(uint32_t timeStampToFill, uint16_t frame_size, uint32_t inputDataLength,
                                                                uint8_t* input, void* correspondingPcmptr)
{
    // normally the gaf audio packet should be consumed after receiving the ack from off mcu core
    bool isConsumeFedGafPacket = true;

    // if m55 encoder init is not successed, it will return false
    if (!gaf_encoder_cc_mcu_env.isGafReceivingJustStarted)
    {
        // m55 core is not ready for encode pcm data
        return false;
    }

    GAF_ENCODER_CC_MEDIA_DATA_T pcm_data_info;
    pcm_data_info.time_stamp = timeStampToFill;
    pcm_data_info.frame_size = frame_size;
    pcm_data_info.data_len   = inputDataLength;
    pcm_data_info.data       = input;
    pcm_data_info.org_addr   = (uint8_t *)correspondingPcmptr;

    gaf_encoder_mcu_accumulate_cmd_send(
                    CROSS_CORE_TASK_CMD_GAF_ENCODE_FEED_PCM_DATA_PACKET, \
                    (uint8_t *)&pcm_data_info);

    return isConsumeFedGafPacket;
}

/**
 ****************************************************************************************
 * @brief When bth received the deinit signal from m55,
 *  it will deinit m55 core according to the para.
 *
 * @param[in] NONE                 NONE
 * @param[in] NONE                 NONE
 *
 * @param[out] NONE                NONE
 ****************************************************************************************
 */
static void gaf_encoder_mcu_deinit_handler(void)
{
    bool is_bis                 = gaf_bth_encoder_recived_deinit_req.is_bis;
    bool is_bis_src             = gaf_bth_encoder_recived_deinit_req.is_bis_src;
    bool is_mobile_role         = gaf_bth_encoder_recived_deinit_req.is_mobile_role;
    bool bth_is_capture_deinit  = gaf_bth_encoder_recived_deinit_req.capture_deinit;
    uint8_t con_lid             = gaf_bth_encoder_recived_deinit_req.con_lid;
    uint32_t context_type       = gaf_bth_encoder_recived_deinit_req.context_type;

    if ((false == is_bis) && (false == is_bis_src)){
        /// single upstream
        if (!is_mobile_role){
            if (true == bth_is_capture_deinit){
                app_bt_start_custom_function_in_bt_thread((uint32_t)con_lid,
                    (uint32_t)context_type, (uint32_t)_gaf_audio_bth_received_encoder_deinit_signal_from_m55);
            }
       }
#ifdef AOB_MOBILE_ENABLED
        else{
            if (true == bth_is_capture_deinit){
                app_bt_start_custom_function_in_bt_thread((uint32_t)con_lid,
                    (uint32_t)context_type, (uint32_t)_gaf_mobile_audio_bth_encoder_received_deinit_signal_from_m55);
            }
        }
#endif
    }
    else if ((true == is_bis) && (true == is_bis_src)){
        if (true == bth_is_capture_deinit){
            app_bt_start_custom_function_in_bt_thread((uint32_t)con_lid,
                    (uint32_t)context_type, (uint32_t)_gaf_audio_bis_bth_encoder_received_deinit_signal_from_m55);
        }
    }
    return ;
}

/***** decoder section ******/
GAF_DECODER_CC_MCU_ENV_T* gaf_mobile_m55_get_decoder_cc_env(void)
{
    // get cc bth rnv
    return &gaf_decoder_cc_mcu_env;
}

static void gaf_decoder_mcu_accumulate_cmd_send(uint16_t cmd_code, uint8_t *data)
{
    uint8_t fetchedData[GAF_DECODER_MAX_LEN_IN_MAILBOX];
    uint16_t lenFetched = 0;
    uint8_t maxAllowCnt = 0;
    uint16_t unitOfInfo = 0;
    uint16_t threshold = GAF_DECODER_MAX_LEN_IN_MAILBOX;
    switch (cmd_code)
    {
        case CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_ENCODED_DATA_PACKET:
            unitOfInfo = sizeof(GAF_DECODER_CC_MEDIA_DATA_T);
        break;
        case CROSS_CORE_TASK_CMD_GAF_DECODE_REMOVE_SPECIFIC_FRAME:
            unitOfInfo = sizeof(GAF_DECODER_CC_FLUSH_T);
        break;
        default:
            ASSERT(0, "%s, Wrong cmdCode:0x%x", __func__, cmd_code);
        break;
    }
    // Accumulate when there are too many mailbox cnts
    //  @APP_DSP_M55_BRIDGE_TX_MAILBOX_MAX
    if (app_dsp_m55_bridge_get_tx_mailbox_cnt() < 5)
    {
        goto send_directly;
    }

    maxAllowCnt = (GAF_DECODER_MAX_LEN_IN_MAILBOX / unitOfInfo);
    threshold = (maxAllowCnt - 1) * unitOfInfo;

    app_dsp_m55_bridge_fetch_tx_mailbox(cmd_code, fetchedData, &lenFetched, threshold);

send_directly:
    memcpy(fetchedData + lenFetched, data, unitOfInfo);
    app_dsp_m55_bridge_send_cmd(cmd_code, fetchedData, lenFetched + unitOfInfo);
}

void gaf_decoder_mcu_remove_specfic_frame(void* frameaddr, uint16_t seq_num)
{
    GAF_DECODER_CC_MEDIA_DATA_T* pMediaData = NULL;

    do
    {
        pMediaData = gaf_decoder_core_peek_from_cached_pcm();
        if (pMediaData)
        {
            // remove its local cached pcm if existing
            uint16_t checkRet = gaf_decoder_cc_check_seq_num(pMediaData->pkt_seq_nb, seq_num);
            if ((GAF_DECODER_SEQ_CHECK_RESULT_MATCH == checkRet) ||
                (GAF_DECODER_SEQ_CHECK_RESULT_EARLIER_THAN_EXPECTED & checkRet))
            {
                DeCQueue(&gaf_decoder_cc_mcu_env.cachedDecodedDataFromCoreQueue, NULL,
                                sizeof(GAF_DECODER_CC_MEDIA_DATA_T));
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    } while (1);

    // remove the cooresponding audio packet and pcm list entry on the m55 side
    GAF_DECODER_CC_FLUSH_T flush_data;
    flush_data.flushFrame     = (uint8_t *)frameaddr;
    flush_data.sequenceNumber = seq_num;
    gaf_media_data_t *head = (gaf_media_data_t*)frameaddr;
    flush_data.channel        = head->cisChannel;

    gaf_decoder_mcu_accumulate_cmd_send(
                        CROSS_CORE_TASK_CMD_GAF_DECODE_REMOVE_SPECIFIC_FRAME, \
                        (uint8_t *)&flush_data);
}

static void gaf_decoder_mcu_init_req_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_instant_cmd_data(CROSS_CORE_INSTANT_CMD_GAF_DECODE_INIT_WAITING_RSP, ptr, len);
}

static void gaf_decoder_mcu_deinit_req_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_with_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_DECODE_DEINIT_WAITING_RSP, ptr, len);
}

/**
 ****************************************************************************************
 * @brief When bth received the deinit decoder signal from m55,
 *  it will deinit m55 core according to the para.
 *
 * @param[in] NONE                 NONE
 * @param[in] NONE                 NONE
 *
 * @param[out] NONE                NONE
 ****************************************************************************************
 */
static void gaf_decoder_mcu_deinit_handler(void)
{
    uint8_t con_lid             = gaf_bth_decoder_recived_deinit_req.con_lid;
    uint32_t context_type       = gaf_bth_decoder_recived_deinit_req.context_type;
    bool bth_is_playback_deinit = gaf_bth_decoder_recived_deinit_req.playback_deinit;
    bool is_bis                 = gaf_bth_decoder_recived_deinit_req.is_bis;
    bool is_bis_src             = gaf_bth_decoder_recived_deinit_req.is_bis_src;
    bool is_mobile_role         = gaf_bth_decoder_recived_deinit_req.is_mobile_role;

    if ((false == is_bis) && (false == is_bis_src)){
        if (!is_mobile_role){
            if (true == bth_is_playback_deinit){
                    app_bt_start_custom_function_in_bt_thread((uint32_t)con_lid,
                        (uint32_t)context_type, (uint32_t)_gaf_audio_bth_received_decoder_deinit_signal_from_m55);
            }
        }
#ifdef AOB_MOBILE_ENABLED
        else
        {
            if (true == bth_is_playback_deinit){
                app_bt_start_custom_function_in_bt_thread((uint32_t)con_lid,
                    (uint32_t)context_type, (uint32_t)_gaf_mobile_audio_bth_decoder_received_deinit_signal_from_m55);
            }
        }
#endif
    }
    else if ((true == is_bis) && (false == is_bis_src)){
        /// For bis, con_id and context_type is not needed, so it was setted to 0
        if (true == bth_is_playback_deinit){
            app_bt_start_custom_function_in_bt_thread((uint32_t)con_lid,
                    (uint32_t)context_type, (uint32_t)_gaf_audio_bis_bth_decoder_received_deinit_signal_from_m55);
        }
    }
}

static void gaf_decoder_mcu_deinit_wait_rsp_timeout_handler(uint8_t* ptr, uint16_t len)
{
    LOG_I("BTH wait for gaf_deinit_rsp from m55 time out!");
    memcpy((uint8_t *)&(gaf_bth_decoder_recived_deinit_req), ptr, sizeof(GAF_AUDIO_M55_DEINIT_T));
    gaf_decoder_mcu_deinit_handler();
}

static void gaf_decoder_mcu_deinit_rsp_received_handler(uint8_t* ptr, uint16_t len)
{
    LOG_I("BTH gets gaf_decoder_deinit_rsp from m55");
    memcpy((uint8_t *)&(gaf_bth_decoder_recived_deinit_req), ptr, sizeof(GAF_AUDIO_M55_DEINIT_T));
    gaf_decoder_mcu_deinit_handler();
}

static void gaf_decoder_mcu_feed_encoded_data_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_ENCODED_DATA_PACKET, ptr, len);
}

static void gaf_decoder_mcu_ack_fed_encoded_data_received_handler(uint8_t* ptr, uint16_t len)
{
    GAF_DECODER_ACK_FEED_PCM_DATA_REQ_T* pReq = (GAF_DECODER_ACK_FEED_PCM_DATA_REQ_T*)ptr;
    // update the decoded and pcm list entry count on the off core side
    gaf_decoder_cc_mcu_env.encodedPacketEntryCount = pReq->encodedPacketEntryCount;
    gaf_decoder_cc_mcu_env.pcmPacketEntryCount = pReq->pcmPacketEntryCount;

    gaf_decoder_cc_mcu_env.cachedPacketCount = gaf_decoder_cc_mcu_env.encodedPacketEntryCount + \
        gaf_decoder_cc_mcu_env.pcmPacketEntryCount;
}

static void gaf_decoder_mcu_retrigger_req_received_handler(uint8_t* param, uint16_t len)
{
    // To do: retrigger audio media
}

static void gaf_decoder_mcu_fed_pcm_data_received_handler(uint8_t* param, uint16_t len)
{
    uint16_t unitOfInfo = sizeof(GAF_DECODER_CC_MEDIA_DATA_T);
    uint8_t packetCnt = len / unitOfInfo;

    while (packetCnt--)
    {
        GAF_DECODER_CC_MEDIA_DATA_T* p_src_media_data_info = (GAF_DECODER_CC_MEDIA_DATA_T *)param;
        LOG_D("rec_fed_pcm_data_len %d", p_src_media_data_info->data_len);
        LOG_D("rec_fed_pcm_seq %d", p_src_media_data_info->pkt_seq_nb);
        uint32_t lock = int_lock();
        int ret = EnCQueue(&gaf_decoder_cc_mcu_env.cachedDecodedDataFromCoreQueue,
            (uint8_t *)p_src_media_data_info, sizeof(GAF_DECODER_CC_MEDIA_DATA_T));
        int_unlock(lock);

        if ((CQ_OK == ret) && p_src_media_data_info->coorespondingEncodedDataPacketPtr)
        {
            // pop from gaf audio list when the pcm data is fed
            if (is_support_ble_audio_mobile_m55_decode == false)
            {
                if (NULL != pLocalStreamEnvPtr)
                {
                    if ((pLocalStreamEnvPtr->stream_context.playback_stream_state < GAF_PLAYBACK_STREAM_INITIALIZED) || (false == is_playback_state))
                    {
                        LOG_I("m55 playback deint or playback stream is not TRIGGERED state!");
                        return ;
                    }
                    gaf_stream_buff_list_t *list_info = NULL;
                    list_info = &(pLocalStreamEnvPtr->stream_context.playback_buff_list[0].buff_list);
                    gaf_list_remove_generic(list_info, p_src_media_data_info->coorespondingEncodedDataPacketPtr);
                }

                if (NULL != pLocalBisStreamEnvPtr)
                {
                    if (pLocalBisStreamEnvPtr->stream_context.playback_stream_state < GAF_PLAYBACK_STREAM_INITIALIZED)
                    {
                        LOG_I("BIS m55 playback deint or playback stream is not TRIGGERED state!");
                        return ;
                    }
                    gaf_stream_buff_list_t *bis_list_info = NULL;
                    bis_list_info = &(pLocalBisStreamEnvPtr->stream_context.playback_buff_list[0].buff_list);
                    gaf_list_remove_generic(bis_list_info, p_src_media_data_info->coorespondingEncodedDataPacketPtr);
                }
            }

#ifdef AOB_MOBILE_ENABLED
            if (is_support_ble_audio_mobile_m55_decode == true)
            {
                if ((NULL != pLocalMobileStreamEnvPtr) && (pLocalMobileStreamEnvPtr->stream_context.playback_stream_state >= GAF_PLAYBACK_STREAM_START_TRIGGERING))
                {
                    gaf_stream_buff_list_t *mobile_list_info = NULL;
                    if (p_src_media_data_info->channel == pLocalMobileStreamEnvPtr->stream_context.playback_buff_list[0].cisChannel)
                    {
                        mobile_list_info = &(pLocalMobileStreamEnvPtr->stream_context.playback_buff_list[0].buff_list);
                    }
                    else
                    {
                        mobile_list_info = &(pLocalMobileStreamEnvPtr->stream_context.playback_buff_list[1].buff_list);
                    }
                    gaf_list_remove_generic(mobile_list_info, p_src_media_data_info->coorespondingEncodedDataPacketPtr);
                }
            }
#endif
        }


        if (CQ_OK != ret)
        {
            LOG_E("local gaf pcm cache overflow: %u",
                LengthOfCQueue(&gaf_decoder_cc_mcu_env.cachedDecodedDataFromCoreQueue)/
                sizeof(GAF_DECODER_CC_MEDIA_DATA_T));
            // To do:force gaf audio trigger
        }

        param += unitOfInfo;
    }
}

static void gaf_decoder_mcu_ack_fed_pcm_data_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_DECODE_ACK_FED_PCM_DATA_PACKET, ptr, len);
}

static void gaf_decoder_mcu_rm_specific_frame_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_DECODE_REMOVE_SPECIFIC_FRAME , ptr, len);
}

void gaf_decoder_core_reset(void)
{
    uint32_t lock = int_lock();

    gaf_decoder_cc_mcu_env.isGafMobileReceivingJustStarted[0] = true;
    gaf_decoder_cc_mcu_env.lastMobileReceivedSeqNum[0] = 0;
    gaf_decoder_cc_mcu_env.isGafMobileReceivingJustStarted[1] = true;
    gaf_decoder_cc_mcu_env.lastMobileReceivedSeqNum[1] = 0;
    gaf_decoder_cc_mcu_env.isGafReceivingJustStarted = true;
    gaf_decoder_cc_mcu_env.lastReceivedSeqNum = 0;
    gaf_decoder_cc_mcu_env.lastReceivedTimeStamp = 0;
    gaf_decoder_cc_mcu_env.encodedPacketEntryCount = 0;
    gaf_decoder_cc_mcu_env.pcmPacketEntryCount = 0;
    gaf_decoder_cc_mcu_env.cachedPacketCount = 0;
    gaf_decoder_cc_mcu_env.offCoreIsReady = false;
    gaf_m55_deinit_status.playback_deinit = false;
    gaf_m55_deinit_status.playback_deinit_sent = false;
    gaf_m55_deinit_status.is_bis          = false;
    gaf_m55_deinit_status.is_mobile_role  = false;
    gaf_bth_decoder_recived_deinit_req.playback_deinit = false;

    InitCQueue(&gaf_decoder_cc_mcu_env.cachedDecodedDataFromCoreQueue,
        sizeof(gaf_decoder_max_cached_encoded_frame_ptr_queue_buf),
        (CQItemType *)gaf_decoder_max_cached_encoded_frame_ptr_queue_buf);

    int_unlock(lock);
}

void gaf_decoder_cc_update_lastframe(gaf_media_data_t* pFrameHeader)
{
    gaf_decoder_cc_mcu_env.lastReceivedSeqNum = pFrameHeader->pkt_seq_nb;
    gaf_decoder_cc_mcu_env.lastReceivedTimeStamp = pFrameHeader->time_stamp;
}

#ifdef LOW_LATENCY_TEST
bool gaf_decoder_mcu_feed_low_latency_encoded_data_into_off_m55_core(
    uint8_t* list_data, uint32_t test_numb,
    uint32_t time_stamp, uint16_t seq_num,
    uint16_t data_len, uint8_t* pData, bool plc)
{
    if (!gaf_decoder_cc_mcu_env.offCoreIsReady) {
        LOG_W("gaf feed_low_latency_encoded_pkt, off core not ready");
        return false;
    }

    GAF_DECODER_CC_MEDIA_DATA_T media_data_info;
    media_data_info.org_addr = list_data;
    media_data_info.time_stamp = time_stamp;
    media_data_info.pkt_seq_nb = seq_num;
    media_data_info.isPlc = plc;
    media_data_info.data_len = data_len;
    media_data_info.data = pData;
    media_data_info.test_seq_numb = test_numb;
    gaf_decoder_mcu_accumulate_cmd_send(
                    CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_ENCODED_DATA_PACKET, \
                    (uint8_t *)&media_data_info);

    return true;
}
#else
bool gaf_decoder_mcu_feed_encoded_data_into_off_m55_core(
    uint8_t* list_data,
    uint32_t time_stamp, uint16_t seq_num,
    uint16_t data_len, uint8_t* pData, bool plc)
{
    if (!gaf_decoder_cc_mcu_env.offCoreIsReady) {
        LOG_W("gaf feed_encoded_pkt, off core not ready");
        return false;
    }

    GAF_DECODER_CC_MEDIA_DATA_T media_data_info;
    media_data_info.org_addr = list_data;
    media_data_info.time_stamp = time_stamp;
    media_data_info.pkt_seq_nb = seq_num;
    media_data_info.isPlc = plc;
    media_data_info.data_len = data_len;
    media_data_info.data = pData;

    gaf_decoder_mcu_accumulate_cmd_send(
                    CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_ENCODED_DATA_PACKET, \
                    (uint8_t *)&media_data_info);

    return true;
}
#endif

void gaf_stream_packet_recover_proc(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, gaf_media_data_t *header)
{
    uint16_t actualSeqNum = header->pkt_seq_nb;
    uint32_t actualTimeStamp = header->time_stamp;
    uint32_t dmaChunkIntervalUs =  pStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
    if (gaf_decoder_cc_mcu_env.isGafReceivingJustStarted)
    {
        gaf_decoder_cc_mcu_env.lastReceivedSeqNum = actualSeqNum - 1;
        gaf_decoder_cc_mcu_env.lastReceivedTimeStamp = actualTimeStamp - dmaChunkIntervalUs;
        gaf_decoder_cc_mcu_env.isGafReceivingJustStarted = false;
    }
    gaf_stream_buff_list_t *list = &pStreamEnv->stream_context.playback_buff_list[0].buff_list;
    LOG_D("rec iso data: len %d list len-> %d seq 0x%x timestamp %u status %d",
         header->data_len, gaf_list_length(list), header->pkt_seq_nb,
         header->time_stamp, header->pkt_status);
    uint16_t diff_frame = actualSeqNum - gaf_decoder_cc_mcu_env.lastReceivedSeqNum;
    if (diff_frame == 1)
    {
        return;
    }
    uint16_t lastSeqNum = gaf_decoder_cc_mcu_env.lastReceivedSeqNum;
    uint32_t lastTimeStamp = gaf_decoder_cc_mcu_env.lastReceivedTimeStamp;
    LOG_W("[ISO] SEQ ERR seq: %d/ %d, timestamp: %d %d", actualSeqNum, lastSeqNum, actualTimeStamp, lastTimeStamp);
    if ((gaf_list_length(list) + diff_frame) <= pStreamEnv->stream_info.playbackInfo.maxCachedEncodedAudioPacketCount)
    {
        for (uint8_t plc_cnt=1; plc_cnt<diff_frame; plc_cnt++)
        {
            gaf_media_data_t *decoder_frame_p = NULL;
            decoder_frame_p = (gaf_media_data_t *)gaf_stream_data_frame_malloc(header->data_len);
            if (NULL == decoder_frame_p)
                return;
            decoder_frame_p->time_stamp = lastTimeStamp + dmaChunkIntervalUs*(uint32_t)plc_cnt;
            decoder_frame_p->pkt_seq_nb = lastSeqNum + plc_cnt;
            decoder_frame_p->pkt_status = header->pkt_status;
            decoder_frame_p->data_len   = header->data_len;
            decoder_frame_p->isPLC      = true;
            LOG_W("[ISO] PLC insert seq %d timestamp %d", decoder_frame_p->pkt_seq_nb, decoder_frame_p->time_stamp);
            gaf_list_append(list, decoder_frame_p);
        }
    }  
}

#ifdef AOB_MOBILE_ENABLED
bool gaf_mobile_decoder_mcu_feed_encoded_data_into_off_m55_core(
    uint8_t* list_data,
    uint32_t time_stamp, uint16_t seq_num,
    uint16_t data_len, uint8_t* pData, uint8_t cis_channel, bool plc)
{
    if (pLocalMobileStreamEnvPtr->stream_context.right_cis_channel == cis_channel){
        gaf_decoder_cc_mcu_env.lastMobileReceivedSeqNum[0] = seq_num;
    }
    else if (pLocalMobileStreamEnvPtr->stream_context.left_cis_channel == cis_channel){
        gaf_decoder_cc_mcu_env.lastMobileReceivedSeqNum[1] = seq_num;
    }

    GAF_DECODER_CC_MEDIA_DATA_T media_data_info;
    if (pLocalMobileStreamEnvPtr->stream_context.right_cis_channel == cis_channel){
       media_data_info.pkt_seq_nb = seq_num;
    }
    else if (pLocalMobileStreamEnvPtr->stream_context.left_cis_channel == cis_channel){
        media_data_info.pkt_seq_nb = seq_num;
    }

    media_data_info.channel    = cis_channel;
    media_data_info.org_addr   = list_data;
    media_data_info.time_stamp = time_stamp;
    media_data_info.isPlc      = plc;
    media_data_info.data_len   = data_len;
    media_data_info.data       = pData;
    gaf_decoder_mcu_accumulate_cmd_send(
                    CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_ENCODED_DATA_PACKET, \
                    (uint8_t *)&media_data_info);

    return true;
}
#endif

static GAF_DECODER_CC_MEDIA_DATA_T* gaf_decoder_core_peek_from_cached_pcm(void)
{
    GAF_DECODER_CC_MEDIA_DATA_T* pRet = NULL;
    uint32_t lock = int_lock();
    LOG_D("cachedDecodedDataFromCoreQueue_len:%d", LengthOfCQueue(&gaf_decoder_cc_mcu_env.cachedDecodedDataFromCoreQueue));
    if (LengthOfCQueue(&gaf_decoder_cc_mcu_env.cachedDecodedDataFromCoreQueue))
    {
        pRet = (GAF_DECODER_CC_MEDIA_DATA_T *)GetCQueueReadPointer(&gaf_decoder_cc_mcu_env.cachedDecodedDataFromCoreQueue);
    }
    int_unlock(lock);
    return pRet;
}

static void gaf_decoder_mcu_ack_feed_pcm_data_to_m55(GAF_DECODER_CC_MEDIA_DATA_T* pMediaPacket)
{
    app_dsp_m55_bridge_send_cmd(
                        CROSS_CORE_TASK_CMD_GAF_DECODE_ACK_FED_PCM_DATA_PACKET, \
                        (uint8_t*)pMediaPacket, \
                        sizeof(GAF_DECODER_CC_MEDIA_DATA_T));
}

static void gaf_audio_decoder_core_pop_oldest_cached_pcm_data(void* _pStreamEnv)
{
    GAF_DECODER_CC_MEDIA_DATA_T* pPopedMediaPacket = NULL;
    uint32_t lock = int_lock();
    if (LengthOfCQueue(&gaf_decoder_cc_mcu_env.cachedDecodedDataFromCoreQueue))
    {
        pPopedMediaPacket = (GAF_DECODER_CC_MEDIA_DATA_T *)GetCQueueReadPointer(&gaf_decoder_cc_mcu_env.cachedDecodedDataFromCoreQueue);
        DeCQueue(&gaf_decoder_cc_mcu_env.cachedDecodedDataFromCoreQueue, NULL,
            sizeof(GAF_DECODER_CC_MEDIA_DATA_T));
    }
    int_unlock(lock);

    if (((GAF_DECODER_CC_MEDIA_DATA_T *)(pPopedMediaPacket->org_addr))->pkt_seq_nb == pPopedMediaPacket->pkt_seq_nb)
    {
        // inform m55 encoded data is fetched
        gaf_decoder_mcu_ack_feed_pcm_data_to_m55(pPopedMediaPacket);
    }
}

static uint16_t gaf_decoder_cc_common_get_packet_status(GAF_AUDIO_STREAM_ENV_T* PStreamEnv,
    uint32_t _dmaIrqHappeningTimeUs, GAF_DECODER_CC_MEDIA_DATA_T* pReadData)
{
    uint16_t isPacketValid = 0;
    int32_t bt_time_diff = 0;
    uint32_t time_stamp = 0;
    int32_t diff_bt_time = 0;

    bt_time_diff = GAF_AUDIO_CLK_32_BIT_DIFF(pReadData->time_stamp, _dmaIrqHappeningTimeUs);

    LOG_D("bt_diff_time:%u  time_stamp:%u  dmairqtime:%u    preDelayus:%u", bt_time_diff,
                    pReadData->time_stamp, _dmaIrqHappeningTimeUs, PStreamEnv->stream_info.playbackInfo.presDelayUs);
#ifdef AOB_MOBILE_ENABLED
    if (is_support_ble_audio_mobile_m55_decode == true)
    {
        if ((bt_time_diff > 0) && ((int32_t)(abs(bt_time_diff) - GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US) > GAF_MOBILE_AUDIO_MAX_DIFF_BT_TIME))
        {
            time_stamp = _dmaIrqHappeningTimeUs - GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US;
            isPacketValid = GAF_DECODER_TIME_CHECK_RESULT_EARLIER_THAN_DMAIRQ;
            LOG_D("received packet's playtime has passed.");
        }
        else if ((bt_time_diff < 0) && ((int32_t)(abs(bt_time_diff) - GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US) > GAF_MOBILE_AUDIO_MAX_DIFF_BT_TIME))
        {
            time_stamp = _dmaIrqHappeningTimeUs - GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US;
            isPacketValid = GAF_DECODER_TIME_CHECK_RESULT_LATER_THAN_DMAIRQ;
            LOG_D("received packet's playtime hasn't arrived.");
        }
        else
        {
            time_stamp = pReadData->time_stamp;
            isPacketValid = GAF_DECODER_TIME_CHECK_RESULT_MATCH_DMAIRQ;
        }

        if (encoded_cnt == 0)
        {
            diff_bt_time = GAF_AUDIO_CLK_32_BIT_DIFF(time_stamp + GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US, _dmaIrqHappeningTimeUs);
            gaf_media_pid_adjust(AUD_STREAM_PLAYBACK, &(PStreamEnv->stream_context.playback_pid_env), diff_bt_time);
            encoded_cnt += 1;
        }
        else
        {
            encoded_cnt -= 1;
        }
    }
#endif
    if (is_support_ble_audio_mobile_m55_decode == false)
    {
        if ((bt_time_diff > 0) && ((int32_t)(abs(bt_time_diff) - PStreamEnv->stream_info.playbackInfo.presDelayUs) > GAF_AUDIO_MAX_DIFF_BT_TIME))
        {
            time_stamp = _dmaIrqHappeningTimeUs - PStreamEnv->stream_info.playbackInfo.presDelayUs;
            isPacketValid = GAF_DECODER_TIME_CHECK_RESULT_EARLIER_THAN_DMAIRQ;
            LOG_D("received packet's playtime has passed.");
        }
        else if ((bt_time_diff < 0) && ((int32_t)(abs(bt_time_diff) - PStreamEnv->stream_info.playbackInfo.presDelayUs) > GAF_AUDIO_MAX_DIFF_BT_TIME))
        {
            time_stamp = _dmaIrqHappeningTimeUs - PStreamEnv->stream_info.playbackInfo.presDelayUs;
            isPacketValid = GAF_DECODER_TIME_CHECK_RESULT_LATER_THAN_DMAIRQ;
            LOG_D("received packet's playtime hasn't arrived.");
        }
        else
        {
            time_stamp = pReadData->time_stamp;
            isPacketValid = GAF_DECODER_TIME_CHECK_RESULT_MATCH_DMAIRQ;
        }

        diff_bt_time = GAF_AUDIO_CLK_32_BIT_DIFF(time_stamp, _dmaIrqHappeningTimeUs) - PStreamEnv->stream_info.playbackInfo.presDelayUs;
        gaf_media_pid_adjust(AUD_STREAM_PLAYBACK, &(PStreamEnv->stream_context.playback_pid_env), diff_bt_time);
    }
    return isPacketValid;
}

static uint16_t gaf_bis_decoder_cc_common_get_packet_status(GAF_AUDIO_STREAM_ENV_T* PStreamEnv,
    uint32_t _dmaIrqHappeningTimeUs, GAF_DECODER_CC_MEDIA_DATA_T* pReadData)
{
    uint16_t isPacketValid = 0;
    int32_t bt_time_diff = 0;
    uint32_t time_stamp = 0;


    uint64_t revlersal_expected_time = 1;
    uint64_t revlersal_current_time = 1;

    bt_time_diff = GAF_AUDIO_CLK_32_BIT_DIFF(pReadData->time_stamp, _dmaIrqHappeningTimeUs);

    LOG_D("bt_diff_time:%u  time_stamp:%u  dmairqtime:%u   preDelayus:%u", bt_time_diff,
                    pReadData->time_stamp, _dmaIrqHappeningTimeUs, PStreamEnv->stream_info.playbackInfo.presDelayUs);


    if ((bt_time_diff > 0) && ((int32_t)(abs(bt_time_diff) - PStreamEnv->stream_info.playbackInfo.presDelayUs) > GAF_AUDIO_MAX_DIFF_BT_TIME))
    {
        time_stamp = _dmaIrqHappeningTimeUs - PStreamEnv->stream_info.playbackInfo.presDelayUs;
        isPacketValid = GAF_DECODER_TIME_CHECK_RESULT_EARLIER_THAN_DMAIRQ;
        LOG_D("received packet's playtime has passed.");
    }
    else if ((bt_time_diff < 0) && ((int32_t)(abs(bt_time_diff) - PStreamEnv->stream_info.playbackInfo.presDelayUs) > GAF_AUDIO_MAX_DIFF_BT_TIME))
    {
        time_stamp = _dmaIrqHappeningTimeUs - PStreamEnv->stream_info.playbackInfo.presDelayUs;
        isPacketValid = GAF_DECODER_TIME_CHECK_RESULT_LATER_THAN_DMAIRQ;
        LOG_D("received packet's playtime hasn't arrived.");
    }
    else
    {
        time_stamp = pReadData->time_stamp;
        isPacketValid = GAF_DECODER_TIME_CHECK_RESULT_MATCH_DMAIRQ;
    }

    int32_t diff_bt_time = 0;

    diff_bt_time = _dmaIrqHappeningTimeUs - time_stamp - PStreamEnv->stream_info.playbackInfo.presDelayUs;

    if (abs(diff_bt_time) > TIME_CALCULATE_REVERSAL_THRESHOLD)
    {
        time_stamp += PStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
        _dmaIrqHappeningTimeUs += PStreamEnv->stream_info.playbackInfo.dmaChunkIntervalUs;
        if (_dmaIrqHappeningTimeUs > time_stamp)
        {
            revlersal_expected_time <<= 32;
            revlersal_expected_time += time_stamp;
            revlersal_current_time = _dmaIrqHappeningTimeUs;
            diff_bt_time = revlersal_current_time - revlersal_expected_time;
        }
        else
        {
            revlersal_current_time <<= 32;
            revlersal_expected_time = time_stamp;
            revlersal_current_time += _dmaIrqHappeningTimeUs;
            diff_bt_time = revlersal_current_time - revlersal_expected_time;
        }
    }
    LOG_I("bth BIS_bt_time_diff:%d", diff_bt_time);
    gaf_media_pid_adjust(AUD_STREAM_PLAYBACK, &(PStreamEnv->stream_context.playback_pid_env), diff_bt_time);
    return isPacketValid;
}

bool gaf_decoder_core_fetch_pcm_data(void* _pStreamEnv, uint16_t expectedSeqNum,
    uint8_t* ptrData, uint16_t dataLen, uint32_t dmaIrqHappeningTimeUs)
{
    uint8_t waitPcmDataCount = 0;
    uint8_t maxwaitPcmDataCount = GAF_MAXIMUM_WAITING_DECODED_DATA_CNT;
    GAF_DECODER_CC_MEDIA_DATA_T *pMediaData = NULL;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    if (true == gaf_bth_decoder_recived_deinit_req.playback_deinit)
    {
        return false;
    }

    if (gaf_stream_get_prefill_status())
    {
        maxwaitPcmDataCount = GAF_MAXIMUM_WAITING_DECODED_DATA_CNT - 1;
    }
    do
    {
peek_again:
        pMediaData = gaf_decoder_core_peek_from_cached_pcm();

        if (NULL == pMediaData)
        {
            LOG_W("cached pcm data queue is empty, wait %d ms until now!",
                waitPcmDataCount*GAF_WAITING_DECODED_DATA_MS_PER_ROUND);
            waitPcmDataCount++;
            if (maxwaitPcmDataCount  >= waitPcmDataCount)
            {
                osDelay(GAF_WAITING_DECODED_DATA_MS_PER_ROUND);
                goto peek_again;
            }
            else
            {
                return false;
            }
        }

        if (false == is_support_ble_audio_mobile_m55_decode)
        {
            expectedSeqNum = pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX];
        }

        LOG_D("bth fetch_pcm_data act_packet_numb:%d expected_numb:%d", pMediaData->pkt_seq_nb, expectedSeqNum);

        uint16_t checkRet = gaf_decoder_cc_check_seq_num(pMediaData->pkt_seq_nb, expectedSeqNum);
        if (GAF_DECODER_SEQ_CHECK_RESULT_MATCH != checkRet)
        {
            LOG_W("fetch_pcm_data SEQ check : %d, seq: %d-%d", checkRet, pMediaData->pkt_seq_nb, expectedSeqNum);
        }

        if (checkRet & GAF_DECODER_SEQ_CHECK_RESULT_DIFF_EXCEEDS_LIMIT)
        {
            // To do: force retriggle media or call
            return false;
        }

        if (GAF_DECODER_SEQ_CHECK_RESULT_MATCH == checkRet)
        {
            // update last frame informastion
            uint16_t isVaildPacket = GAF_DECODER_TIME_CHECK_RESULT_MATCH_DMAIRQ;
            if (!gaf_stream_get_prefill_status())
            {
                isVaildPacket = gaf_decoder_cc_common_get_packet_status(pStreamEnv,
                        dmaIrqHappeningTimeUs, pMediaData);
            }
            if (GAF_DECODER_TIME_CHECK_RESULT_MATCH_DMAIRQ != isVaildPacket)
            {
                LOG_W("fetch_pcm_data TIME check : %d", isVaildPacket);
            }
            if (GAF_DECODER_TIME_CHECK_RESULT_MATCH_DMAIRQ == isVaildPacket)
            {
                uint32_t lock = int_lock();
                memcpy(ptrData, pMediaData->data, dataLen);
                int_unlock(lock);
                gaf_audio_decoder_core_pop_oldest_cached_pcm_data(pStreamEnv);
            }
            else if (GAF_DECODER_TIME_CHECK_RESULT_EARLIER_THAN_DMAIRQ == isVaildPacket)
            {
                if (true == is_support_ble_audio_mobile_m55_decode)
                {
                    if (pStreamEnv->stream_context.right_cis_channel == pMediaData->channel)
                    {
                        pStreamEnv->stream_context.lastestPlaybackSeqNumR++;
                    }
                    else if (pStreamEnv->stream_context.left_cis_channel == pMediaData->channel)
                    {
                        pStreamEnv->stream_context.lastestPlaybackSeqNumL++;
                    }
                    gaf_audio_decoder_core_pop_oldest_cached_pcm_data(pStreamEnv);
                }
                else
                {
                    pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]++;
                    gaf_audio_decoder_core_pop_oldest_cached_pcm_data(pStreamEnv);
                }
                goto peek_again;
            }
            else if (GAF_DECODER_TIME_CHECK_RESULT_LATER_THAN_DMAIRQ == isVaildPacket)
            {
                uint32_t lock = int_lock();
                memset(ptrData, 0, dataLen);
                int_unlock(lock);
            }
        }
        else if (GAF_DECODER_SEQ_CHECK_RESULT_LATER_THAN_EXPECTED == checkRet)
        {
            uint32_t lock = int_lock();
            memset(ptrData, 0, dataLen);
            int_unlock(lock);
        }
        else if (GAF_DECODER_SEQ_CHECK_RESULT_EARLIER_THAN_EXPECTED == checkRet)
        {
            // discard the frame
            gaf_audio_decoder_core_pop_oldest_cached_pcm_data(pStreamEnv);
            goto peek_again;
        }
        else
        {
            break;
        }
    } while(0);
    return true;
}

bool gaf_bis_decoder_core_fetch_pcm_data(void* _pStreamEnv, uint16_t expectedSeqNum,
    uint8_t* ptrData, uint16_t dataLen, uint32_t dmaIrqHappeningTimeUs)
{
    uint8_t waitPcmDataCount = 0;

    GAF_DECODER_CC_MEDIA_DATA_T *pMediaData = NULL;
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    if (true == gaf_bth_decoder_recived_deinit_req.playback_deinit)
    {
        return false;
    }

    do
    {
peek_again:
        pMediaData = gaf_decoder_core_peek_from_cached_pcm();

        if (NULL == pMediaData)
        {
            LOG_W("cached pcm data queue is empty, wait %d ms until now!",
                waitPcmDataCount*GAF_WAITING_DECODED_DATA_MS_PER_ROUND);
            waitPcmDataCount++;
            if (GAF_MAXIMUM_WAITING_DECODED_DATA_CNT >= waitPcmDataCount)
            {
                osDelay(GAF_WAITING_DECODED_DATA_MS_PER_ROUND);
                goto peek_again;
            }
            else
            {
                return false;
            }
        }

        LOG_I("%s   packet_numb:%d    expected_numb:%d", __func__, pMediaData->pkt_seq_nb, expectedSeqNum);
        uint16_t checkRet = gaf_decoder_cc_check_seq_num(pMediaData->pkt_seq_nb, expectedSeqNum);

        if (checkRet & GAF_DECODER_SEQ_CHECK_RESULT_DIFF_EXCEEDS_LIMIT)
        {
            // To do: force retriggle media or call
            return false;
        }

        if (GAF_DECODER_SEQ_CHECK_RESULT_MATCH == checkRet)
        {
            // update last frame informastion
            uint16_t isVaildPacket = gaf_bis_decoder_cc_common_get_packet_status(pStreamEnv,
                    dmaIrqHappeningTimeUs, pMediaData);

            if (GAF_DECODER_TIME_CHECK_RESULT_MATCH_DMAIRQ == isVaildPacket)
            {
                memcpy(ptrData, pMediaData->data, dataLen);
                gaf_audio_decoder_core_pop_oldest_cached_pcm_data(pStreamEnv);
            }
            else if (GAF_DECODER_TIME_CHECK_RESULT_EARLIER_THAN_DMAIRQ == isVaildPacket)
            {
                gaf_audio_decoder_core_pop_oldest_cached_pcm_data(pStreamEnv);
                pStreamEnv->stream_context.lastestPlaybackSeqNum[GAF_AUDIO_DFT_PLAYBACK_LIST_IDX]++;
                goto peek_again;
            }
            else if (GAF_DECODER_TIME_CHECK_RESULT_LATER_THAN_DMAIRQ == isVaildPacket)
            {
                memcpy(ptrData, pMediaData->data, dataLen);
            }
        }
        else if (GAF_DECODER_SEQ_CHECK_RESULT_LATER_THAN_EXPECTED == checkRet)
        {
            // fill the fake frame
            memcpy(ptrData, pMediaData->data, dataLen);
        }
        else if (GAF_DECODER_SEQ_CHECK_RESULT_EARLIER_THAN_EXPECTED == checkRet)
        {
            // discard the frame
            gaf_audio_decoder_core_pop_oldest_cached_pcm_data(pStreamEnv);
            goto peek_again;
        }
        else
        {
            break;
        }
    } while(0);
    return true;
}
#endif
