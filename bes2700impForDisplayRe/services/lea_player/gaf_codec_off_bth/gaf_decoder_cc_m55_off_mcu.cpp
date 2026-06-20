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
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
#include "cmsis.h"
#include "cmsis_os.h"
#include "plat_types.h"
#include "cqueue.h"
#include "list.h"
#include "heap_api.h"
#include "hal_sysfreq.h"
#include "hal_trace.h"
#include "gaf_codec_cc_common.h"
#include "app_dsp_m55.h"
#include "app_gaf_dbg.h"
#include "app_utils.h"

/************************private macro defination***************************/
#define GAF_CODEC_M55_DECODER_PROCESSOR_THREAD_STACK_SIZE            (1024*4)
#define GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED            (1 << 0)
#define GAF_CODEC_M55_DECODER_THREAD_SIGNAL_STOP                     (1 << 1)
#define GAF_CODEC_M55_DECODER_THREAD_SIGNAL_START                    (1 << 2)
#define GAF_CODEC_M55_DECODER_THREAD_SIGNAL_REMOVE_SPECIFIC_FRAME    (1 << 3)
#define GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_RIGHT_CHL  (1 << 4)
#define GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_LEFT_CHL   (1 << 5)
#define GAF_DECODER_CC_MAX_ENCODED_DATA_CNT_LIMITER                  (24)
/// this max value is considered for the bit_rate 12800, sample_rate is 48000
/// The acculate equal: encoded_len = bit_rate * frame_ms (interval_ms) * 10 / 80000;
#define GAF_DECODER_CC_MAX_ENCODED_DATA_LEN_LIMITER                  (160)
// when the pcm data entry list has up to this number, m55 will pasue the encoding until
// the count of the decoding data is consumed down
#define GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER                      (8)
/// this max value is considered for sample_rate is 48000, 10ms interval,
/// 24bit_width, 1920 + offset value
#define GAF_DECODER_CC_MAX_PCM_DATA_LEN_LIMITER                      (2000)
// shall increase this value if memory pool is not enough
#define GAF_MOBILE_DECODER_CC_MEM_POOL_SIZE                          (((GAF_DECODER_CC_MAX_ENCODED_DATA_CNT_LIMITER) * 1024) + \
                                                                     ((GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER) * 1200) + \
                                                                     (GAF_DECODER_CC_MAX_ENCODED_DATA_CNT_LIMITER + GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER) * \
                                                                     sizeof(GAF_DECODER_CC_MEDIA_DATA_T))
#ifdef BLE_AUDIO_STEREO_CHANNEL_OVER_ONE_CIS
#define GAF_DECODER_CC_MEM_POOL_SIZE                                 2 * (((GAF_DECODER_CC_MAX_ENCODED_DATA_CNT_LIMITER) * GAF_DECODER_CC_MAX_ENCODED_DATA_LEN_LIMITER) + \
                                                                     ((GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER) * GAF_DECODER_CC_MAX_PCM_DATA_LEN_LIMITER) + \
                                                                     (GAF_DECODER_CC_MAX_ENCODED_DATA_CNT_LIMITER + GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER) * \
                                                                     sizeof(GAF_DECODER_CC_MEDIA_DATA_T))
#else
#define GAF_DECODER_CC_MEM_POOL_SIZE                                 (((GAF_DECODER_CC_MAX_ENCODED_DATA_CNT_LIMITER) * GAF_DECODER_CC_MAX_ENCODED_DATA_LEN_LIMITER) + \
                                                                     ((GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER) * GAF_DECODER_CC_MAX_PCM_DATA_LEN_LIMITER) + \
                                                                     (GAF_DECODER_CC_MAX_ENCODED_DATA_CNT_LIMITER + GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER) * \
                                                                     sizeof(GAF_DECODER_CC_MEDIA_DATA_T))
#endif
#define MAXIMUM_PENDING_REMOVED_DECODED_PACKET_CNT                   (64)
#define GAF_DECODER_M55_MAX_LEN_IN_MAILBOX                           (512)

/************************private type defination****************************/
typedef struct
{
    void        *decode_mutex;
    list_t      *decode_list;
    list_node_t *decode_node;
} GAF_DECODER_CC_BUFF_LIST_T;

typedef struct
{
    GAF_DECODER_CC_BUFF_LIST_T      right_channel_encoded_data_list;
    GAF_DECODER_CC_BUFF_LIST_T      right_channel_decoded_data_list;
    GAF_DECODER_CC_BUFF_LIST_T      left_channel_encoded_data_list;
    GAF_DECODER_CC_BUFF_LIST_T      left_channel_decoded_data_list;
    bool                            isMobileGafStreamingOnGoing[2];

    GAF_DECODER_CC_BUFF_LIST_T      encoded_data_list;
    GAF_DECODER_CC_BUFF_LIST_T      decoded_data_list;
    // const values once the GAF is started
    bool                            isGafStreamingOnGoing;
    uint8_t                         m55_right_cis_channel;
    uint8_t                         m55_left_cis_channel;
    GAF_DECODER_INIT_REQ_T          cachedInitReq;
    GAF_AUDIO_M55_DEINIT_T          cachedDeinitReq;

    CQueue                          toBeRemovedEncodedPacketQueue;

    int32_t                         channels;
    uint16_t                        maxPcmDataSizePerFrame;
    uint16_t                        frame_len;
    GAF_AUDIO_CC_DECODER_T         audio_decoder;
} GAF_DECODER_CC_M55_OFF_MCU_ENV_T;

/*********************external variable declaration*************************/
extern GAF_AUDIO_CC_DECODER_T gaf_audio_cc_lc3_decoder_config;
bool is_support_ble_audio_mobile_m55_decode_cc_core = false;
extern uint8_t channel_value;
uint8_t right_cis_channel = 0;
uint8_t left_cis_channel = 0;

/************************private variable defination************************/
heap_handle_t gaf_decode_m55_heap_handle = NULL;
GAF_DECODER_CC_M55_OFF_MCU_ENV_T gaf_decoder_cc_m55_off_mcu_env;
static uint8_t *gaf_decoder_cc_mem_pool= NULL;
static osThreadId gaf_codec_m55_decode_processor_thread_id = NULL;
static GAF_DECODER_CC_FLUSH_T gaf_decoder_cached_to_be_removed_frame_ptr_queue_buf[MAXIMUM_PENDING_REMOVED_DECODED_PACKET_CNT];
static enum APP_SYSFREQ_FREQ_T gaf_decoder_cc_base_freq = APP_SYSFREQ_30M;

/**********************private function declaration*************************/
static void gaf_decoder_cc_init_req_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_decode_m55_deinit_req_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_decode_m55_ack_feed_encoded_data_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_m55_rm_specific_frame_handler(void);
static void gaf_decode_m55_feed_encoded_data_req_received_handler(uint8_t* param, uint16_t len);
static void gaf_decoder_m55_retrigger_req_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_m55_feed_pcm_data_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_m55_ack_fed_pcm_data_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_decoder_m55_rm_specific_frame_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_codec_m55_decode_processor_thread(const void *arg);
static void gaf_decoder_cc_init_rsp_transit_handler(uint8_t* ptr, uint16_t len);

osThreadDef(gaf_codec_m55_decode_processor_thread, osPriorityAboveNormal, 1,
    (GAF_CODEC_M55_DECODER_PROCESSOR_THREAD_STACK_SIZE), "gaf_codec_m55_decode_processor_thread");
osMutexDef(gaf_decoder_cc_encoded_data_buff_mutex);
osMutexDef(gaf_decoder_cc_pcm_data_buffer_mutex);

M55_CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(CROSS_CORE_INSTANT_CMD_GAF_DECODE_INIT_WAITING_RSP,
                            NULL,
                            gaf_decoder_cc_init_req_received_handler);

M55_CORE_BRIDGE_INSTANT_COMMAND_TO_ADD(CROSS_CORE_INSTANT_CMD_GAF_DECODE_INIT_RSP_TO_CORE,
                            gaf_decoder_cc_init_rsp_transit_handler,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_DEINIT_WAITING_RSP,
                            "CC_GAF_DECODE_DEINIT_NO_RSP",
                            NULL,
                            gaf_decode_m55_deinit_req_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_ENCODED_DATA_PACKET,
                            "CC_FEED_ENCODED_DATA_PACKET",
                            NULL,
                            gaf_decode_m55_feed_encoded_data_req_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_ACK_FEED_ENCODED_DATA_PACKET,
                            "CC_ACK_FEED_ENCODED_DATA_PACKET",
                            gaf_decode_m55_ack_feed_encoded_data_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_RETRIGGER_REQ_NO_RSP,
                            "CC_GAF_RETRIGGER_REQ_NO_RSP",
                            gaf_decoder_m55_retrigger_req_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_PCM_DATA_PACKET,
                            "CC_FEED_PCM_DATA_PACKET",
                            gaf_decoder_m55_feed_pcm_data_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_ACK_FED_PCM_DATA_PACKET,
                            "CC_ACK_FEED_PCM_DATA_PACKET",
                            NULL,
                            gaf_decoder_m55_ack_fed_pcm_data_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_DECODE_REMOVE_SPECIFIC_FRAME,
                            "GAF_REMOVE_SPECIFIC_FRAME",
                            NULL,
                            gaf_decoder_m55_rm_specific_frame_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

/****************************function defination****************************/
static void gaf_decoder_m55_accumulate_cmd_send(uint16_t cmd_code, uint8_t *data)
{
    uint8_t fetchedData[GAF_DECODER_M55_MAX_LEN_IN_MAILBOX];
    uint16_t lenFetched = 0;
    uint8_t maxAllowCnt = 0;
    uint16_t unitOfInfo = 0;
    uint16_t threshold = GAF_DECODER_M55_MAX_LEN_IN_MAILBOX;
    switch(cmd_code)
    {
        case CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_PCM_DATA_PACKET:
            unitOfInfo = sizeof(GAF_DECODER_CC_MEDIA_DATA_T);
        break;
        default:
            ASSERT(0, "%s, Wrong cmdCode:0x%x", __func__, cmd_code);
        break;
    }
    // Accumulate when there are too many mailbox cnts
    // @APP_DSP_M55_BRIDGE_TX_MAILBOX_MAX
    if (app_dsp_m55_bridge_get_tx_mailbox_cnt() < 5)
    {
        goto send_directly;
    }

    maxAllowCnt = (GAF_DECODER_M55_MAX_LEN_IN_MAILBOX / unitOfInfo);
    threshold = (maxAllowCnt - 1) * unitOfInfo;

    app_dsp_m55_bridge_fetch_tx_mailbox(cmd_code, fetchedData, &lenFetched, threshold);
send_directly:
    memcpy(fetchedData + lenFetched, data, unitOfInfo);
    app_dsp_m55_bridge_send_cmd(cmd_code, fetchedData, lenFetched + unitOfInfo);
}

static void gaf_decoder_cc_init_rsp_transit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_instant_cmd_data(CROSS_CORE_INSTANT_CMD_GAF_DECODE_INIT_RSP_TO_CORE, ptr, len);
}

static void gaf_decoder_cc_m55_heap_init(void *begin_addr, uint32_t size)
{
    LOG_I("init heap %d", size);
    gaf_decode_m55_heap_handle = heap_register(begin_addr, size);
}

static void gaf_decoder_cc_m55_stream_reset(void)
{
    InitCQueue(&gaf_decoder_cc_m55_off_mcu_env.toBeRemovedEncodedPacketQueue,
        sizeof(gaf_decoder_cached_to_be_removed_frame_ptr_queue_buf),
        (CQItemType *)gaf_decoder_cached_to_be_removed_frame_ptr_queue_buf);
}

static int inline gaf_decoder_cc_buffer_mutex_lock(void *mutex)
{
    osMutexWait((osMutexId)mutex, osWaitForever);
    return 0;
}

static int inline gaf_decoder_cc_buffer_mutex_unlock(void *mutex)
{
    osMutexRelease((osMutexId)mutex);
    return 0;
}

static void gaf_decoder_cc_heap_free(void *rmem)
{
    ASSERT(rmem, "%s rmem:%p", __func__, rmem);
    uint32_t lock = int_lock();
    heap_free(gaf_decode_m55_heap_handle, rmem);
    int_unlock(lock);
}

static void gaf_decoder_cc_data_free(void *packet)
{
    ASSERT(packet, "%s packet = %p", __func__, packet);
    GAF_DECODER_CC_MEDIA_DATA_T *decoder_frame_p = (GAF_DECODER_CC_MEDIA_DATA_T *)packet;
    if (decoder_frame_p->data)
    {
        gaf_decoder_cc_heap_free(decoder_frame_p->data);
    }
    gaf_decoder_cc_heap_free(decoder_frame_p);
}

static void *gaf_decoder_cc_heap_cmalloc(uint32_t size)
{
    uint32_t lock = int_lock();
    size = ((size >> 2) + 1) << 2;
    void *ptr = heap_malloc(gaf_decode_m55_heap_handle, size);
    ASSERT(ptr, "%s size:%d", __func__, size);
    memset(ptr, 0, size);
    int_unlock(lock);

    return ptr;
}

static void *gaf_decoder_cc_heap_malloc(uint32_t size)
{
    uint32_t lock = int_lock();
    size = ((size >> 2) + 1) << 2;
    void *ptr = heap_malloc(gaf_decode_m55_heap_handle, size);
    ASSERT(ptr, "%s size:%d", __func__, size);
    int_unlock(lock);

    return ptr;
}

static GAF_DECODER_CC_MEDIA_DATA_T *gaf_decoder_cc_data_frame_malloc(uint32_t packet_len)
{
    GAF_DECODER_CC_MEDIA_DATA_T *decoder_frame_p = NULL;
    uint8_t *buffer = NULL;

    if (packet_len)
    {
        buffer = (uint8_t *)gaf_decoder_cc_heap_malloc(packet_len);
    }
    decoder_frame_p = (GAF_DECODER_CC_MEDIA_DATA_T *)gaf_decoder_cc_heap_malloc(sizeof(GAF_DECODER_CC_MEDIA_DATA_T));
    decoder_frame_p->org_addr = (uint8_t *)decoder_frame_p;
    decoder_frame_p->data = buffer;
    decoder_frame_p->data_len = packet_len;
    return decoder_frame_p;
}

static list_node_t *gaf_decoder_cc_list_begin(GAF_DECODER_CC_BUFF_LIST_T *list_info)
{
    gaf_decoder_cc_buffer_mutex_lock(list_info->decode_mutex);
    list_node_t *node = list_begin(list_info->decode_list);
    gaf_decoder_cc_buffer_mutex_unlock(list_info->decode_mutex);
    return node;
}

static void *gaf_decoder_cc_list_node(GAF_DECODER_CC_BUFF_LIST_T *list_info)
{
    gaf_decoder_cc_buffer_mutex_lock(list_info->decode_mutex);
    void *data = list_node(list_info->decode_node);
    gaf_decoder_cc_buffer_mutex_unlock(list_info->decode_mutex);
    return data;
}

static bool gaf_decoder_cc_list_append(GAF_DECODER_CC_BUFF_LIST_T *list_info, void *data)
{
    gaf_decoder_cc_buffer_mutex_lock(list_info->decode_mutex);
    bool nRet = list_append(list_info->decode_list, data);
    gaf_decoder_cc_buffer_mutex_unlock(list_info->decode_mutex);
    return nRet;
}

static uint32_t gaf_decoder_cc_list_length(GAF_DECODER_CC_BUFF_LIST_T *list_info)
{
    gaf_decoder_cc_buffer_mutex_lock(list_info->decode_mutex);
    uint32_t length = list_length(list_info->decode_list);
    gaf_decoder_cc_buffer_mutex_unlock(list_info->decode_mutex);
    return length;
}

static void gaf_decoder_cc_list_new(GAF_DECODER_CC_BUFF_LIST_T *list_info, const osMutexDef_t *mutex_def,
    list_free_cb callback, list_mempool_zmalloc zmalloc, list_mempool_free free)
{
    if (NULL == list_info->decode_mutex)
    {
        list_info->decode_mutex = osMutexCreate(mutex_def);
    }
    ASSERT(list_info->decode_mutex, "%s decoder mutex create failed", __func__);

    gaf_decoder_cc_buffer_mutex_lock(list_info->decode_mutex);
    list_info->decode_list = list_new(callback, zmalloc, free);
    gaf_decoder_cc_buffer_mutex_unlock(list_info->decode_mutex);
    LOG_I("decoder new LIST 0x%x -> 0x%x", (uint32_t)list_info, (uint32_t)list_info->decode_list);
}

static bool gaf_decoder_cc_list_remove(GAF_DECODER_CC_BUFF_LIST_T *list_info, void *data)
{
    gaf_decoder_cc_buffer_mutex_lock(list_info->decode_mutex);
    bool nRet = list_remove(list_info->decode_list, data);
    gaf_decoder_cc_buffer_mutex_unlock(list_info->decode_mutex);
    return nRet;
}

static void gaf_decoder_cc_init_req_received_handler(uint8_t* ptr, uint16_t len)
{
    if (!gaf_decoder_cc_m55_off_mcu_env.isGafStreamingOnGoing)
    {
        // Avoid timing issue
        memcpy((uint8_t *)&(gaf_decoder_cc_m55_off_mcu_env.cachedInitReq), ptr, sizeof(GAF_DECODER_INIT_REQ_T));
        osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_START);
    }
}

static void gaf_decode_m55_deinit_req_received_handler(uint8_t* ptr, uint16_t len)
{
    // inform the m55 processor thread
    memcpy((uint8_t *)&(gaf_decoder_cc_m55_off_mcu_env.cachedDeinitReq), ptr, sizeof(GAF_AUDIO_M55_DEINIT_T));
    osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_STOP);
}

static void gaf_decode_m55_ack_feed_encoded_data_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_DECODE_ACK_FEED_ENCODED_DATA_PACKET, ptr, len);
}

static void gaf_decoder_m55_retrigger_req_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_DECODE_RETRIGGER_REQ_NO_RSP, ptr, len);
}

static void gaf_decoder_m55_feed_pcm_data_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_PCM_DATA_PACKET, ptr, len);
}

static void gaf_decoder_m55_ack_fed_pcm_data_received_handler(uint8_t* ptr, uint16_t len)
{
    GAF_DECODER_CC_MEDIA_DATA_T* pMediaData = (GAF_DECODER_CC_MEDIA_DATA_T *)ptr;
    if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
    {
        if (gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel == pMediaData->channel){
            if (!gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[0]){
                return;
            }
            gaf_decoder_cc_list_remove(&gaf_decoder_cc_m55_off_mcu_env.right_channel_decoded_data_list,
                (void *)(pMediaData->org_addr));
            // inform the processor thread
            osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_RIGHT_CHL);
        }
        else if (gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel == pMediaData->channel){
            if (!gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[1]){
                return;
            }
            gaf_decoder_cc_list_remove(&gaf_decoder_cc_m55_off_mcu_env.left_channel_decoded_data_list,
                (void *)(pMediaData->org_addr));
            // inform the processor thread
            osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_LEFT_CHL);
        }
    }
    else
    {
        if (!gaf_decoder_cc_m55_off_mcu_env.isGafStreamingOnGoing)
        {
            return;
        }
        gaf_decoder_cc_list_remove(&gaf_decoder_cc_m55_off_mcu_env.decoded_data_list,
                (void *)(pMediaData->org_addr));

        // inform the processor thread
        osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED);
    }
}

static void gaf_decoder_m55_rm_specific_frame_received_handler(uint8_t* ptr, uint16_t len)
{
    if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
    {
        GAF_DECODER_CC_FLUSH_T *pMedia = (GAF_DECODER_CC_FLUSH_T* )ptr;
        if (gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel == pMedia->channel){
            if (!gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[0]){
                LOG_I("gaf stream has already been stopped:%s", __func__);
                return;
            }
        }

        if (gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel == pMedia->channel){
            if (!gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[1]){
                LOG_I("gaf stream has already been stopped:%s", __func__);
                return;
            }
        }

    }
    else
    {
        if (!gaf_decoder_cc_m55_off_mcu_env.isGafStreamingOnGoing)
        {
            LOG_I("gaf stream has already been stopped:%s", __func__);
            return;
        }
    }

    uint32_t lock = int_lock();
    int ret = EnCQueue(&gaf_decoder_cc_m55_off_mcu_env.toBeRemovedEncodedPacketQueue, (uint8_t *)ptr, len);
    int_unlock(lock);

    ASSERT(CQ_OK == ret, "toBeRemovedEncodedPacketQueue overflow!!!");
    osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_REMOVE_SPECIFIC_FRAME);
}


POSSIBLY_UNUSED static void gaf_decoder_cc_ack_feed_encoded_data_to_core(GAF_DECODER_ACK_FEED_PCM_DATA_REQ_T* pReq)
{
   app_dsp_m55_bridge_send_cmd(
                    CROSS_CORE_TASK_CMD_GAF_DECODE_ACK_FEED_ENCODED_DATA_PACKET, \
                    (uint8_t*)pReq, \
                    sizeof(GAF_DECODER_ACK_FEED_PCM_DATA_REQ_T));
}

static void gaf_decode_m55_feed_encoded_data_req_received_handler(uint8_t* param, uint16_t len)
{
    bool isGoing = false;
    bool isInformRChReceived = false;
    bool isInformLChReceived = false;
    bool isInformPacketReceived = false;
    uint16_t unitOfInfo = sizeof(GAF_DECODER_CC_MEDIA_DATA_T);
    uint8_t packetCnt = len / unitOfInfo;
    GAF_DECODER_CC_MEDIA_DATA_T* p_src_media_data_info = NULL;

    while (packetCnt--)
    {
        p_src_media_data_info = (GAF_DECODER_CC_MEDIA_DATA_T*)param;

        /// recording the mobile m55 right channel and left channel value
        if (gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel == 0)
        {
            gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel = p_src_media_data_info->channel;
            right_cis_channel = gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel;
        }
        else if (   (gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel == 0)
                && (gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel != 0)
                && (gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel != gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel))
        {
            gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel = p_src_media_data_info->channel;
            left_cis_channel = gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel;
        }

        if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
        {
            if (gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel == p_src_media_data_info->channel)
            {
                isGoing = gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[0];
            }
            else if (gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel == p_src_media_data_info->channel)
            {
                isGoing = gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[1];
            }
        }
        else
        {
            isGoing = gaf_decoder_cc_m55_off_mcu_env.isGafStreamingOnGoing;
        }

        if (!isGoing)
        {
            LOG_I("gaf stream has already been stopped.:%s", __func__);
        }
        else
        {
            GAF_DECODER_CC_MEDIA_DATA_T* p_stored_media_data_info =
                gaf_decoder_cc_data_frame_malloc(gaf_decoder_cc_m55_off_mcu_env.frame_len);
            p_stored_media_data_info->isPlc      = p_src_media_data_info->isPlc;
            p_stored_media_data_info->pkt_seq_nb = p_src_media_data_info->pkt_seq_nb;
            p_stored_media_data_info->time_stamp = p_src_media_data_info->time_stamp;
            if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
            {
                p_stored_media_data_info->channel = p_src_media_data_info->channel;
            }
            p_stored_media_data_info->coorespondingEncodedDataPacketPtr = (void *)p_src_media_data_info->org_addr;

            // copy to local buffer for decoding
            memcpy(p_stored_media_data_info->data, p_src_media_data_info->data, p_src_media_data_info->data_len);

            if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
            {
                if (gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel == p_stored_media_data_info->channel)
                {
                    gaf_decoder_cc_list_append(&gaf_decoder_cc_m55_off_mcu_env.right_channel_encoded_data_list,
                        p_stored_media_data_info);
                    // send notification to mcu core based on the m55 thread signal
                    if (!isInformRChReceived)
                    {
                        if (gaf_decoder_cc_list_length(&gaf_decoder_cc_m55_off_mcu_env.right_channel_decoded_data_list)
                            < GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER)
                        {
                            isInformRChReceived = true;
                            // inform the processor thread
                            osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_RIGHT_CHL);
                        }
                    }
                }
                else if (gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel == p_stored_media_data_info->channel)
                {
                    gaf_decoder_cc_list_append(&gaf_decoder_cc_m55_off_mcu_env.left_channel_encoded_data_list,
                        p_stored_media_data_info);
                    // send notification to mcu core based on the m55 thread signal
                    if (!isInformLChReceived)
                    {
                        if (gaf_decoder_cc_list_length(&gaf_decoder_cc_m55_off_mcu_env.left_channel_decoded_data_list)
                            < GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER)
                        {
                            isInformLChReceived = true;
                            // inform the processor thread
                            osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_LEFT_CHL);
                        }
                    }
                }
            }
            else
            {
                gaf_decoder_cc_list_append(&gaf_decoder_cc_m55_off_mcu_env.encoded_data_list,
                p_stored_media_data_info);

                if (!isInformPacketReceived)
                {
                    // send notification to mcu core based on the m55 thread signal
                    if (gaf_decoder_cc_list_length(&gaf_decoder_cc_m55_off_mcu_env.decoded_data_list)
                        < GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER)
                    {
                        isInformPacketReceived = true;
                        // inform the processor thread
                        osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED);
                    }
                }
            }
        }
        param += unitOfInfo;
    }
}

static bool gaf_decoder_cc_rm_node_with_specific_audio_packet(
    GAF_DECODER_CC_BUFF_LIST_T *list_info, void* pFlushedFrame, uint16_t seq)
{
    bool isFound = false;
    list_node_t *node = NULL;
    GAF_DECODER_CC_MEDIA_DATA_T* pListEntry;
    // should be able to find on the firs entry
    // get the decode list node head
    gaf_decoder_cc_buffer_mutex_lock(list_info->decode_mutex);
peek_again:
    node = list_begin(list_info->decode_list);
    if (node != list_end(list_info->decode_list))
    {
        pListEntry = (GAF_DECODER_CC_MEDIA_DATA_T *)list_node(node);
        if (!pListEntry) {
            goto exit;
        }

        LOG_I("RemovepListEntry,%p-%p, seq:%u-%u",
            pListEntry->coorespondingEncodedDataPacketPtr, pFlushedFrame,
            pListEntry->pkt_seq_nb, seq);

        if ((pListEntry->pkt_seq_nb < seq) || (ABS((int32_t)pListEntry->pkt_seq_nb - (int32_t)seq) > 32)) {
            list_remove(list_info->decode_list, list_node(node));
            goto peek_again;
        }

        if ((pListEntry->coorespondingEncodedDataPacketPtr == pFlushedFrame) &&
            (pListEntry->pkt_seq_nb == seq))
        {
            isFound = true;
            list_remove(list_info->decode_list, list_node(node));
        }
    }
exit:
    gaf_decoder_cc_buffer_mutex_unlock(list_info->decode_mutex);
    return isFound;
}

static void gaf_decoder_m55_rm_specific_frame_handler(void)
{
    GAF_DECODER_CC_FLUSH_T pFlushedData;
    do
    {
        uint32_t lock = int_lock();
        int ret = DeCQueue(&gaf_decoder_cc_m55_off_mcu_env.toBeRemovedEncodedPacketQueue,
            (uint8_t *)&pFlushedData, sizeof(GAF_DECODER_CC_FLUSH_T));
        int_unlock(lock);
        if (CQ_OK == ret)
        {
            if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
            {
                if (gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel == pFlushedData.channel){
                    gaf_decoder_cc_rm_node_with_specific_audio_packet(
                        &gaf_decoder_cc_m55_off_mcu_env.right_channel_decoded_data_list,
                        pFlushedData.flushFrame, pFlushedData.sequenceNumber);

                    gaf_decoder_cc_rm_node_with_specific_audio_packet(
                        &gaf_decoder_cc_m55_off_mcu_env.right_channel_encoded_data_list,
                        pFlushedData.flushFrame, pFlushedData.sequenceNumber);
                }
                else if (gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel == pFlushedData.channel){
                    gaf_decoder_cc_rm_node_with_specific_audio_packet(
                        &gaf_decoder_cc_m55_off_mcu_env.left_channel_decoded_data_list,
                        pFlushedData.flushFrame, pFlushedData.sequenceNumber);

                    gaf_decoder_cc_rm_node_with_specific_audio_packet(
                        &gaf_decoder_cc_m55_off_mcu_env.left_channel_encoded_data_list,
                        pFlushedData.flushFrame, pFlushedData.sequenceNumber);
                }
            }
            else
            {
                gaf_decoder_cc_rm_node_with_specific_audio_packet(
                    &gaf_decoder_cc_m55_off_mcu_env.decoded_data_list,
                    pFlushedData.flushFrame, pFlushedData.sequenceNumber);

                gaf_decoder_cc_rm_node_with_specific_audio_packet(
                    &gaf_decoder_cc_m55_off_mcu_env.encoded_data_list,
                    pFlushedData.flushFrame, pFlushedData.sequenceNumber);
            }
        }
        else
        {
            break;
        }
    } while (1);

    if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
    {
        if (gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel == pFlushedData.channel){
            if (gaf_decoder_cc_list_length(&gaf_decoder_cc_m55_off_mcu_env.right_channel_encoded_data_list) > 0)
            {
                osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_RIGHT_CHL);
            }
        }
        else if (gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel == pFlushedData.channel){
            if (gaf_decoder_cc_list_length(&gaf_decoder_cc_m55_off_mcu_env.left_channel_encoded_data_list) > 0)
            {
                osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_LEFT_CHL);
            }
        }
    }
    else
    {
        if (gaf_decoder_cc_list_length(&gaf_decoder_cc_m55_off_mcu_env.encoded_data_list) > 0)
        {
            osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED);
        }
    }
}

POSSIBLY_UNUSED static void gaf_decoder_cc_retrigger_mcu_core_audio_streaming(void)
{
    app_dsp_m55_bridge_send_cmd(
                CROSS_CORE_TASK_CMD_GAF_DECODE_RETRIGGER_REQ_NO_RSP, \
                NULL, 0);
}

static void gaf_decoder_cc_boost_timeout_handler(void const *n);
osTimerDef (GAF_DECODER_CC_FREQ_BOOST_TIMER, gaf_decoder_cc_boost_timeout_handler);
osTimerId gaf_decoder_cc_freq_boost_timerid = NULL;
#define GAF_DECODER_CC_FREQ_BOOST_TIMEOUT_VALUE  (1500)

static void gaf_decoder_cc_boost_timeout_handler(void const *n)
{
    TRACE(0,"gaf_decoder_cc_boost_timeout_handler");
    if (gaf_decoder_cc_m55_off_mcu_env.isGafStreamingOnGoing)
    {
        app_sysfreq_req(APP_SYSFREQ_USER_AOB_PLAYBACK, gaf_decoder_cc_base_freq);
    }
}

void gaf_decoder_cc_start_boost_freq(void)
{
    if (NULL == gaf_decoder_cc_freq_boost_timerid)
    {
        gaf_decoder_cc_freq_boost_timerid = osTimerCreate(osTimer(GAF_DECODER_CC_FREQ_BOOST_TIMER), osTimerOnce, NULL);
    }

    app_sysfreq_req(APP_SYSFREQ_USER_AOB_PLAYBACK, APP_SYSFREQ_208M);
    osTimerStop(gaf_decoder_cc_freq_boost_timerid);
    osTimerStart(gaf_decoder_cc_freq_boost_timerid, GAF_DECODER_CC_FREQ_BOOST_TIMEOUT_VALUE);
}

static void gaf_decoder_cc_m55_init_handler(void)
{
    bool clearMem = false;
    LOG_I("[%s] start", __func__);
    GAF_DECODER_INIT_REQ_T* pReq = &(gaf_decoder_cc_m55_off_mcu_env.cachedInitReq);
    gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel  = 0;
    gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel   = 0;
    gaf_decoder_cc_m55_off_mcu_env.channels = pReq->stream_cfg.num_channels;
    gaf_decoder_cc_m55_off_mcu_env.maxPcmDataSizePerFrame = pReq->maxPcmDataSizePerFrame;
    gaf_decoder_cc_m55_off_mcu_env.frame_len = pReq->stream_cfg.frame_size;
    if (true == pReq->is_support_audio_mobile)
    {
        is_support_ble_audio_mobile_m55_decode_cc_core = true;
    }
    else
    {
        is_support_ble_audio_mobile_m55_decode_cc_core = false;
    }

    LOG_I("INIT_num_channels:%d", pReq->stream_cfg.num_channels);
    LOG_I("INIT_bits_depth:%d", pReq->stream_cfg.bits_depth);
    LOG_I("INIT_bitrate:%d", pReq->stream_cfg.bitrate);
    LOG_I("INIT_sample_rate:%d", pReq->stream_cfg.sample_rate);
    LOG_I("INIT_frame_len:%d", pReq->stream_cfg.frame_size);
    LOG_I("is central:%d", is_support_ble_audio_mobile_m55_decode_cc_core);

    gaf_decoder_cc_m55_stream_reset();
    if (is_support_ble_audio_mobile_m55_decode_cc_core == false)
    {
        if (NULL == gaf_decoder_cc_mem_pool)
        {
            off_bth_syspool_get_buff(&gaf_decoder_cc_mem_pool, GAF_DECODER_CC_MEM_POOL_SIZE);
            ASSERT(gaf_decoder_cc_mem_pool, "%s size:%d", __func__, GAF_DECODER_CC_MEM_POOL_SIZE);
            clearMem = true;
        }
        gaf_decoder_cc_m55_heap_init(gaf_decoder_cc_mem_pool, GAF_DECODER_CC_MEM_POOL_SIZE);
    }
    else
    {
        if (NULL == gaf_decoder_cc_mem_pool)
        {
            off_bth_syspool_get_buff(&gaf_decoder_cc_mem_pool, GAF_MOBILE_DECODER_CC_MEM_POOL_SIZE);
            ASSERT(gaf_decoder_cc_mem_pool, "%s size:%d", __func__, GAF_MOBILE_DECODER_CC_MEM_POOL_SIZE);
            clearMem = true;
        }
        gaf_decoder_cc_m55_heap_init(gaf_decoder_cc_mem_pool, GAF_MOBILE_DECODER_CC_MEM_POOL_SIZE);
    }

    if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
    {
        gaf_decoder_cc_list_new(&gaf_decoder_cc_m55_off_mcu_env.right_channel_encoded_data_list,
                (osMutex(gaf_decoder_cc_encoded_data_buff_mutex)),
                gaf_decoder_cc_data_free,
                gaf_decoder_cc_heap_cmalloc,
                gaf_decoder_cc_heap_free);

        gaf_decoder_cc_list_new(&gaf_decoder_cc_m55_off_mcu_env.right_channel_decoded_data_list,
                (osMutex(gaf_decoder_cc_pcm_data_buffer_mutex)),
                gaf_decoder_cc_data_free,
                gaf_decoder_cc_heap_cmalloc,
                gaf_decoder_cc_heap_free);

        gaf_decoder_cc_list_new(&gaf_decoder_cc_m55_off_mcu_env.left_channel_encoded_data_list,
                (osMutex(gaf_decoder_cc_encoded_data_buff_mutex)),
                gaf_decoder_cc_data_free,
                gaf_decoder_cc_heap_cmalloc,
                gaf_decoder_cc_heap_free);

        gaf_decoder_cc_list_new(&gaf_decoder_cc_m55_off_mcu_env.left_channel_decoded_data_list,
                (osMutex(gaf_decoder_cc_pcm_data_buffer_mutex)),
                gaf_decoder_cc_data_free,
                gaf_decoder_cc_heap_cmalloc,
                gaf_decoder_cc_heap_free);
    }
    else
    {
        gaf_decoder_cc_list_new(&gaf_decoder_cc_m55_off_mcu_env.encoded_data_list,
                (osMutex(gaf_decoder_cc_encoded_data_buff_mutex)),
                gaf_decoder_cc_data_free,
                gaf_decoder_cc_heap_cmalloc,
                gaf_decoder_cc_heap_free);

        gaf_decoder_cc_list_new(&gaf_decoder_cc_m55_off_mcu_env.decoded_data_list,
                (osMutex(gaf_decoder_cc_pcm_data_buffer_mutex)),
                gaf_decoder_cc_data_free,
                gaf_decoder_cc_heap_cmalloc,
                gaf_decoder_cc_heap_free);
    }

    GAF_AUDIO_CC_DECODER_T *p_gaf_audio_cc_decoder = NULL;
    p_gaf_audio_cc_decoder = &gaf_audio_cc_lc3_decoder_config;

    // setup m55 decode frequency
    enum APP_SYSFREQ_FREQ_T gaf_decoder_m55_freq = gaf_decoder_cc_base_freq;

    gaf_decoder_cc_base_freq = gaf_audio_stream_decoder_adjust_m55_freq(pReq->stream_cfg.frame_size,
        pReq->stream_cfg.num_channels, pReq->stream_cfg.frame_ms,
        pReq->stream_cfg.sample_rate, gaf_decoder_m55_freq,
        pReq->stream_playback_state, pReq->stream_capture_state);

    gaf_decoder_cc_start_boost_freq();

    memcpy((uint8_t *)&p_gaf_audio_cc_decoder->stream_config,
        (uint8_t *)&pReq->stream_cfg,
        sizeof(GAF_AUDIO_CC_DECODE_OUTPUT_T));

    memcpy((uint8_t *)&gaf_decoder_cc_m55_off_mcu_env.audio_decoder,
            (uint8_t *)p_gaf_audio_cc_decoder,
            sizeof(GAF_AUDIO_CC_DECODER_T));
    if (clearMem) {
        gaf_decoder_cc_m55_off_mcu_env.audio_decoder.gaf_audio_decoder_cc_clear_buffer_pointer();
    }

    gaf_decoder_cc_m55_off_mcu_env.audio_decoder.gaf_audio_decoder_cc_init();

    if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
    {
        gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[0] = true;
        gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[1] = true;
    }
    else
    {
        gaf_decoder_cc_m55_off_mcu_env.isGafStreamingOnGoing = true;
    }

    GAF_AUDIO_DECODER_INIT_RSP_T rsp;
    app_dsp_m55_bridge_send_cmd(CROSS_CORE_INSTANT_CMD_GAF_DECODE_INIT_RSP_TO_CORE,
        (uint8_t *)&rsp, sizeof(rsp));
    LOG_I("[%s] end", __func__);
}

static void gaf_decoder_cc_feed_pcm_data_to_core(GAF_DECODER_CC_MEDIA_DATA_T* pMediaData)
{
    gaf_decoder_m55_accumulate_cmd_send(CROSS_CORE_TASK_CMD_GAF_DECODE_FEED_PCM_DATA_PACKET, (uint8_t*)pMediaData);
}

static void gaf_decoder_cc_process_encoding(GAF_DECODER_CC_MEDIA_DATA_T* pMediaPacket)
{
    GAF_DECODER_CC_MEDIA_DATA_T* p_stored_pcm_data_info =
        gaf_decoder_cc_data_frame_malloc(gaf_decoder_cc_m55_off_mcu_env.maxPcmDataSizePerFrame);
    if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
    {
        p_stored_pcm_data_info->channel = pMediaPacket->channel;
    }

    p_stored_pcm_data_info->isPlc = pMediaPacket->isPlc;
    p_stored_pcm_data_info->pkt_seq_nb = pMediaPacket->pkt_seq_nb;
    p_stored_pcm_data_info->time_stamp = pMediaPacket->time_stamp;
    p_stored_pcm_data_info->coorespondingEncodedDataPacketPtr =
        pMediaPacket->coorespondingEncodedDataPacketPtr;
    uint32_t encodedata_len = (uint32_t)pMediaPacket->data_len;
    uint8_t  *encodedata    = pMediaPacket->data;

    channel_value = p_stored_pcm_data_info->channel;
    if (p_stored_pcm_data_info->isPlc)
    {
        TRACE(0, "[LC3 Decoder] PLC seqnum %d timestamp %d", p_stored_pcm_data_info->pkt_seq_nb, p_stored_pcm_data_info->time_stamp);
    }

    gaf_decoder_cc_m55_off_mcu_env.audio_decoder.gaf_audio_decoder_cc_pcm_frame(encodedata_len, encodedata,
            p_stored_pcm_data_info->data, p_stored_pcm_data_info->isPlc);


    if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
    {
        if (gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel == p_stored_pcm_data_info->channel){
            gaf_decoder_cc_list_append(&gaf_decoder_cc_m55_off_mcu_env.right_channel_decoded_data_list, p_stored_pcm_data_info);
        }
        else if (gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel == p_stored_pcm_data_info->channel){
            gaf_decoder_cc_list_append(&gaf_decoder_cc_m55_off_mcu_env.left_channel_decoded_data_list, p_stored_pcm_data_info);
        }
    }
    else
    {
        gaf_decoder_cc_list_append(&gaf_decoder_cc_m55_off_mcu_env.decoded_data_list, p_stored_pcm_data_info);
    }

    // send pcm data to mcu core
    gaf_decoder_cc_feed_pcm_data_to_core(p_stored_pcm_data_info);
}

static void gaf_decoder_cc_process_encoded_data_right_channel(void)
{
    GAF_DECODER_CC_BUFF_LIST_T *list = &gaf_decoder_cc_m55_off_mcu_env.right_channel_encoded_data_list;
    GAF_DECODER_CC_MEDIA_DATA_T* decoded_frame_p = NULL;

    uint32_t currentPcmListLen = gaf_decoder_cc_list_length(
        &gaf_decoder_cc_m55_off_mcu_env.right_channel_decoded_data_list);

    if (currentPcmListLen >= GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER)
    {
        return;
    }

    do
    {
        list->decode_node = gaf_decoder_cc_list_begin(list);
        if (NULL == list->decode_node)
        {
            LOG_D("gaf_decoder_playback_list_is_empty:%d", 0);
            break;
        }

        decoded_frame_p = (GAF_DECODER_CC_MEDIA_DATA_T *)gaf_decoder_cc_list_node(list);

        gaf_decoder_cc_process_encoding(decoded_frame_p);
        gaf_decoder_cc_list_remove(list, decoded_frame_p);
    } while (0);

    if (gaf_decoder_cc_list_length(list) > 0)
    {
        osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_RIGHT_CHL);
    }
}

static void gaf_decoder_cc_process_encoded_data_left_channel(void)
{
    GAF_DECODER_CC_BUFF_LIST_T *list = &gaf_decoder_cc_m55_off_mcu_env.left_channel_encoded_data_list;
    GAF_DECODER_CC_MEDIA_DATA_T* decoded_frame_p = NULL;

    uint32_t currentPcmListLen = gaf_decoder_cc_list_length(
        &gaf_decoder_cc_m55_off_mcu_env.left_channel_decoded_data_list);

    if (currentPcmListLen >= GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER)
    {
        return;
    }

    do
    {
        list->decode_node = gaf_decoder_cc_list_begin(list);
        if (NULL == list->decode_node)
        {
            LOG_D("gaf_decoder_playback_list_is_empty:%d", 0);
            break;
        }

        decoded_frame_p = (GAF_DECODER_CC_MEDIA_DATA_T *)gaf_decoder_cc_list_node(list);

        gaf_decoder_cc_process_encoding(decoded_frame_p);
        gaf_decoder_cc_list_remove(list, decoded_frame_p);

    } while (0);

    if (gaf_decoder_cc_list_length(list) > 0)
    {
        osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_LEFT_CHL);
    }
}

static void gaf_decoder_cc_process_encoded_data(void)
{
    GAF_DECODER_CC_BUFF_LIST_T *list = &gaf_decoder_cc_m55_off_mcu_env.encoded_data_list;
    GAF_DECODER_CC_MEDIA_DATA_T* decoded_frame_p = NULL;

    uint32_t currentPcmListLen = gaf_decoder_cc_list_length(
        &gaf_decoder_cc_m55_off_mcu_env.decoded_data_list);

    if (currentPcmListLen >= GAF_DECODER_CC_MAX_PCM_DATA_CNT_LIMITER)
    {
        return;
    }

    do
    {
        list->decode_node = gaf_decoder_cc_list_begin(list);
        if (NULL == list->decode_node)
        {
            LOG_D("gaf_decoder_playback_list_is_empty:%d", 0);
            break;
        }

        decoded_frame_p = (GAF_DECODER_CC_MEDIA_DATA_T *)gaf_decoder_cc_list_node(list);

        gaf_decoder_cc_process_encoding(decoded_frame_p);
        gaf_decoder_cc_list_remove(list, decoded_frame_p);

    } while (0);

    if (gaf_decoder_cc_list_length(list) > 0)
    {
        osSignalSet(gaf_codec_m55_decode_processor_thread_id, GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED);
    }
}

static void gaf_decoder_cc_off_bth_syspool_cb(void)
{
    gaf_decoder_cc_mem_pool = NULL;
}

static void gaf_codec_m55_decode_processor_thread(const void *arg)
{
    while(1)
    {
        osEvent evt;
        // wait any signal
        evt = osSignalWait(0x0, osWaitForever);

        // get role from signal value
        if (osEventSignal == evt.status)
        {
            // get start signal to init decoder
            if (evt.value.signals & GAF_CODEC_M55_DECODER_THREAD_SIGNAL_START)
            {
                off_bth_syspool_init(SYSPOOL_USER_GAF_DECODE, gaf_decoder_cc_off_bth_syspool_cb);
                gaf_decoder_cc_m55_init_handler();
            }

            // get stop signal to deinit decoder
            if (evt.value.signals & GAF_CODEC_M55_DECODER_THREAD_SIGNAL_STOP)
            {
                if ((gaf_decoder_cc_m55_off_mcu_env.isGafStreamingOnGoing) ||
                    ((gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[0] == true) &&
                     (gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[1] == true)))
                {
                    gaf_decoder_cc_m55_off_mcu_env.audio_decoder.gaf_audio_decoder_cc_deinit();
                    gaf_decoder_cc_m55_stream_reset();
                    if (true == is_support_ble_audio_mobile_m55_decode_cc_core)
                    {
                        gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[0] = false;
                        gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[1] = false;
                        gaf_decoder_cc_m55_off_mcu_env.m55_left_cis_channel  = 0;
                        gaf_decoder_cc_m55_off_mcu_env.m55_right_cis_channel = 0;
                        right_cis_channel = 0;
                        left_cis_channel = 0;
                    }
                    else
                    {
                        gaf_decoder_cc_m55_off_mcu_env.isGafStreamingOnGoing = false;
                    }

                    app_sysfreq_req(APP_SYSFREQ_USER_AOB_PLAYBACK, APP_SYSFREQ_32K);

                    off_bth_syspool_deinit(SYSPOOL_USER_GAF_DECODE);
                }

                /// feed decoder_deinit signal to bth, when m55 finished the deinit
                GAF_AUDIO_M55_DEINIT_T m55_decoder_deinit_rsp;
                m55_decoder_deinit_rsp.capture_deinit   =
                    gaf_decoder_cc_m55_off_mcu_env.cachedDeinitReq.capture_deinit;
                m55_decoder_deinit_rsp.playback_deinit  =
                    gaf_decoder_cc_m55_off_mcu_env.cachedDeinitReq.playback_deinit;
                m55_decoder_deinit_rsp.is_bis           =
                    gaf_decoder_cc_m55_off_mcu_env.cachedDeinitReq.is_bis;
                m55_decoder_deinit_rsp.is_bis_src       =
                    gaf_decoder_cc_m55_off_mcu_env.cachedDeinitReq.is_bis_src;
                m55_decoder_deinit_rsp.is_mobile_role   =
                    gaf_decoder_cc_m55_off_mcu_env.cachedDeinitReq.is_mobile_role;
                m55_decoder_deinit_rsp.con_lid          =
                    gaf_decoder_cc_m55_off_mcu_env.cachedDeinitReq.con_lid;
                m55_decoder_deinit_rsp.context_type     =
                    gaf_decoder_cc_m55_off_mcu_env.cachedDeinitReq.context_type;

                app_dsp_m55_bridge_send_rsp(CROSS_CORE_TASK_CMD_GAF_DECODE_DEINIT_WAITING_RSP, \
                                            (uint8_t *)&m55_decoder_deinit_rsp, \
                                            sizeof(GAF_AUDIO_M55_DEINIT_T));
            }

            // get start signal to process encoded data
            if (evt.value.signals & GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED)
            {
                if (gaf_decoder_cc_m55_off_mcu_env.isGafStreamingOnGoing)
                {
                    gaf_decoder_cc_process_encoded_data();
                }
            }

            // mobile get start signal to process encoded data
            if (evt.value.signals & GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_RIGHT_CHL)
            {
                if (gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[0])
                {
                    gaf_decoder_cc_process_encoded_data_right_channel();
                }
            }
            if (evt.value.signals & GAF_CODEC_M55_DECODER_THREAD_SIGNAL_DATA_RECEIVED_LEFT_CHL)
            {
                if (gaf_decoder_cc_m55_off_mcu_env.isMobileGafStreamingOnGoing[1])
                {
                    gaf_decoder_cc_process_encoded_data_left_channel();
                }
            }

            if (evt.value.signals & GAF_CODEC_M55_DECODER_THREAD_SIGNAL_REMOVE_SPECIFIC_FRAME)
            {
                gaf_decoder_m55_rm_specific_frame_handler();
            }
        }
    }
}

void gaf_codec_decoder_m55_init(void)
{
    if (NULL == gaf_codec_m55_decode_processor_thread_id)
    {
        gaf_codec_m55_decode_processor_thread_id =
        osThreadCreate(osThread(gaf_codec_m55_decode_processor_thread), NULL);
    }
}
#endif
