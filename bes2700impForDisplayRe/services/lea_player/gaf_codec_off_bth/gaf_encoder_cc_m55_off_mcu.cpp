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
#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "plat_types.h"
#include "cqueue.h"
#include "list.h"
#include "heap_api.h"
#include "audio_dump.h"
#include "hal_sysfreq.h"
#include "app_dsp_m55.h"
#include "app_gaf_dbg.h"
#include "gaf_codec_cc_common.h"
#include "app_utils.h"
#ifdef SCO_DSP_ACCEL
#include "iir_resample.h"
#include "speech_algo.h"
#include "speech_algo_utils.h"
#include "speech_algo_dsp_rpc.h"
#endif

/************************private macro defination***************************/
#define GAF_CODEC_M55_PROCESSOR_THREAD_STACK_SIZE            (1024*4)
#define GAF_CODEC_M55_ENCODER_THREAD_SIGNAL_DATA_RECEIVED    (1 << 0)
#define GAF_CODEC_M55_THREAD_SIGNAL_STOP                     (1 << 1)
#define GAF_CODEC_M55_THREAD_SIGNAL_START                    (1 << 2)
#define GAF_ENCODER_CC_MAX_ENCODED_DATA_CNT_LIMITER          (8)
/// At present, it need to consider that speech CALL algorithm,
/// the max value is considered for four mic channel + one vpu
/// , sample_rate is 32000, bit_width is 24, and 10ms interval, bit_rate is 96000
/// The acculate equal: encoded_len = bit_rate * frame_ms (interval_ms) * 10 / 80000;
#define GAF_ENCODER_CC_MAX_ENCODED_DATA_LEN_CNT_LIMITER      (600)
/// when the pcm data entry list has up to this number, pasue the encoding until
/// the count of the pcm data is consumed down
#define GAF_ENCODER_CC_MAX_PCM_DATA_CNT_LIMITER              (12)
/// This max value is considered for four mic channel + one vpu, sample_rate is 32000,
/// bit_width is 24, and 10ms interval, bit_rate is 96000
/// The acculate equal is pcm_data_len = sample_rate * bit_width/8 * frame_ms * channel_num/(1000 * 1000);
#define GAF_ENCODER_CC_MAX_PCM_DATA_LEN_LIMITER              (6400)
// shall increase this value if memory pool is not enough
#define GAF_ENCODER_CC_MEM_POOL_SIZE                         (((GAF_ENCODER_CC_MAX_ENCODED_DATA_CNT_LIMITER) * GAF_ENCODER_CC_MAX_ENCODED_DATA_LEN_CNT_LIMITER) + \
                                                              ((GAF_ENCODER_CC_MAX_PCM_DATA_CNT_LIMITER) * GAF_ENCODER_CC_MAX_PCM_DATA_LEN_LIMITER) + \
                                                              (GAF_ENCODER_CC_MAX_ENCODED_DATA_CNT_LIMITER + GAF_ENCODER_CC_MAX_PCM_DATA_CNT_LIMITER) * \
                                                               sizeof(GAF_ENCODER_CC_MEDIA_DATA_T))
#define MAXIMUM_PENDING_REMOVED_ENCODED_PACKET_CNT           (64)
#define GAF_ENCODER_M55_MAX_LEN_IN_MAILBOX                   (512)

// #define CALC_MIPS_CNT_THD_MS        (3000)

/************************private type defination****************************/
typedef struct
{
    void        *encode_mutex;
    list_t      *encode_list;
    list_node_t *encode_node;
} gaf_encoder_cc_buff_list_t;

typedef struct
{
    // changing values
    uint8_t                         isEncodingJustStarted;
    uint16_t                        lastEncodedSeqNum;
    uint16_t                        lastEncodedSubSeqNum;
    GAF_ENCODER_INIT_REQ_T          cachedInitReq;
    GAF_AUDIO_M55_DEINIT_T          cachedDeinitReq;

    gaf_encoder_cc_buff_list_t      encoded_data_list;
    gaf_encoder_cc_buff_list_t      pcm_data_list;

    CQueue                          toBeAddedDecodedPacketQueue;

    // const value once the gaf audio codec is started
    bool                            isGafStreamingOnGoing;
    int32_t                         channels;
    uint16_t                        frame_size;
    uint16_t                        maxEncodedDataSizePerFrame;

    GAF_AUDIO_CC_ENCODER_T          gaf_audio_encoder;
}GAF_ENCODER_CC_M55_OFF_MCU_ENV_T;

/************************private variable defination************************/
extern GAF_AUDIO_CC_ENCODER_T gaf_audio_cc_lc3_encoder_config;
static heap_handle_t gaf_encode_m55_heap_handle = NULL;
static GAF_ENCODER_CC_M55_OFF_MCU_ENV_T gaf_codec_cc_m55_off_mcu_env;
static osThreadId gaf_codec_m55_encode_processor_thread_id = NULL;
static uint8_t *gaf_encoder_cc_mem_pool = NULL;
static GAF_ENCODER_CC_FLUSH_T gaf_encoder_cached_to_be_removed_frame_ptr_queue_buf[MAXIMUM_PENDING_REMOVED_ENCODED_PACKET_CNT];
static enum APP_SYSFREQ_FREQ_T gaf_encoder_cc_base_freq = APP_SYSFREQ_120M;

#ifdef SCO_DSP_ACCEL
#define GAF_TX_ALGO_RUN_ON_SPEECH_THREAD
// #define GAF_TX_ALGO_UPSAMPLING_DUMP
#define TX_ALGO_UPSAMPLING_WORKAROUND
static bool g_tx_algo_enabled = false;
static uint32_t g_tx_algo_in_pcm_len = 0;
static int32_t *g_tx_algo_out_pcm_buf_ptr = NULL;
static int32_t *g_tx_algo_input_pcm_buf_ptr[PCM_CHANNEL_NUM] = {NULL};
static uint32_t g_tx_algo_upsampling_factor = 1;
#ifdef TX_ALGO_UPSAMPLING_WORKAROUND
static bool g_tx_algo_upsampling_enabled = false;
#else
// There is a crash issue happen when call end. Crash is related with multi_iir_resample_init
// Need to debug whether it's related with CMSIS IIR Filter error issue.
static IirResampleState *g_tx_algo_upsampling_st = NULL;
#endif
static uint8_t *gaf_speech_heap_buf = NULL;
#endif /// SCO_DSP_ACCEL

/**********************private function declaration*************************/
static void gaf_encoder_cc_process_encoding(GAF_ENCODER_CC_MEDIA_DATA_T* pMediaPacket);
static void gaf_encoder_cc_m55_stream_reset(void);
static void gaf_encoder_cc_m55_init_handler(void);
static void gaf_encoder_cc_pcm_data_received_handler(uint8_t* para, uint16_t len);
static void gaf_encode_m55_init_req_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_encode_m55_deinit_req_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_codec_m55_encode_processor_thread(const void *arg);
static bool gaf_encoder_cc_list_remove(gaf_encoder_cc_buff_list_t *list_info, void *data);
static void gaf_ecoder_cc_ack_feed_encoded_data_received_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_cc_retrigger_req_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_cc_feed_encoded_data_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_cc_ack_fed_pcm_data_transmit_handler(uint8_t* ptr, uint16_t len);
static void gaf_encoder_cc_ack_feed_pcm_data_successed_transmit_handler(uint8_t* ptr, uint16_t len);

osThreadDef(gaf_codec_m55_encode_processor_thread, osPriorityAboveNormal, 1,
    (GAF_CODEC_M55_PROCESSOR_THREAD_STACK_SIZE), "gaf_encoder_m55_processor_thread");
osMutexDef(gaf_encoder_cc_encoded_data_buff_mutex);
osMutexDef(gaf_encoder_cc_pcm_data_buff_mutex);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ENCODE_INIT_WAITING_RSP,
                            "CC_GAF_ENCODE_INIT_WAITING_RSP",
                            NULL,
                            gaf_encode_m55_init_req_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ENCODE_DEINIT_WAITING_RSP,
                            "CC_GAF_ENCODE_DEINIT_NO_RSP",
                            NULL,
                            gaf_encode_m55_deinit_req_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_RETRIGGER_REQ_NO_RSP,
                            "CC_GAF_RETRIGGLE_REQ_NO_RSP",
                            gaf_encoder_cc_retrigger_req_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ACK_FEED_ENCODED_DATA_TO_CORE,
                            "CC_GAF_ACK_FEED_PCM_DATA_PACKET_TO_ENCODE",
                            NULL,
                            gaf_ecoder_cc_ack_feed_encoded_data_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_FEED_ENCODED_DATA_TO_CORE,
                            "FEED_ENCODED_DATA",
                            gaf_encoder_cc_feed_encoded_data_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ENCODE_FEED_PCM_DATA_PACKET,
                            "CC_GAF_FEED_PCM_DATA_PACKET_TO_ENCODE",
                            NULL,
                            gaf_encoder_cc_pcm_data_received_handler,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ACK_ENCODE_FEED_PCM_DATA_PACKET,
                            "ACK_FED_PCM_DATA_PACKET",
                            gaf_encoder_cc_ack_fed_pcm_data_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

RPC_CROSS_CORE_TASK_CMD_TO_ADD(CROSS_CORE_TASK_CMD_GAF_ACK_FEED_PCM_DATA_SUCCEED,
                            "ACK_FEED_PCM_DATA_PACKET_SUCCESSED",
                            gaf_encoder_cc_ack_feed_pcm_data_successed_transmit_handler,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            NULL);

/****************************function defination****************************/

static void gaf_encoder_m55_accumulate_cmd_send(uint16_t cmd_code, uint8_t *data)
{
    uint8_t fetchedData[GAF_ENCODER_M55_MAX_LEN_IN_MAILBOX];
    uint16_t lenFetched = 0;
    uint8_t maxAllowCnt = 0;
    uint16_t unitOfInfo = 0;
    uint16_t threshold = GAF_ENCODER_M55_MAX_LEN_IN_MAILBOX;

    switch(cmd_code)
    {
        case CROSS_CORE_TASK_CMD_GAF_FEED_ENCODED_DATA_TO_CORE:
            unitOfInfo = sizeof(GAF_ENCODER_CC_MEDIA_DATA_T);
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

    maxAllowCnt = (GAF_ENCODER_M55_MAX_LEN_IN_MAILBOX / unitOfInfo);
    threshold = (maxAllowCnt - 1) * unitOfInfo;

    app_dsp_m55_bridge_fetch_tx_mailbox(cmd_code, fetchedData, &lenFetched, threshold);
send_directly:
    memcpy(fetchedData + lenFetched, data, unitOfInfo);
    app_dsp_m55_bridge_send_cmd(cmd_code, fetchedData, lenFetched + unitOfInfo);
}


static void gaf_encoder_cc_m55_heap_init(void *begin_addr, uint32_t size)
{
    gaf_encode_m55_heap_handle = heap_register(begin_addr, size);
}

static int inline gaf_encoder_cc_buffer_mutex_lock(void *mutex)
{
    osMutexWait((osMutexId)mutex, osWaitForever);
    return 0;
}

static int inline gaf_encoder_cc_buffer_mutex_unlock(void *mutex)
{
    osMutexRelease((osMutexId)mutex);
    return 0;
}

static void gaf_encoder_cc_heap_free(void *rmem)
{
    ASSERT(rmem, "%s rmem:%p", __func__, rmem);
    uint32_t lock = int_lock();
    heap_free(gaf_encode_m55_heap_handle, rmem);
    int_unlock(lock);
}

static void gaf_encoder_cc_data_free(void *packet)
{
    ASSERT(packet, "%s packet = %p", __func__, packet);
    GAF_ENCODER_CC_MEDIA_DATA_T *encoder_frame_p = (GAF_ENCODER_CC_MEDIA_DATA_T *)packet;
    if (encoder_frame_p->data)
    {
        gaf_encoder_cc_heap_free(encoder_frame_p->data);
    }
    gaf_encoder_cc_heap_free(encoder_frame_p);
}

static void *gaf_encoder_cc_heap_malloc(uint32_t size)
{
    uint32_t lock = int_lock();
    size = ((size >> 2) + 1) << 2;
    void *ptr = heap_malloc(gaf_encode_m55_heap_handle, size);
    ASSERT(ptr, "%s size:%d", __func__, size);
    int_unlock(lock);
    return ptr;
}

static void *gaf_encoder_cc_heap_cmalloc(uint32_t size)
{
    uint32_t lock = int_lock();
    size = ((size >> 2) + 1) << 2;
    void *ptr = heap_malloc(gaf_encode_m55_heap_handle, size);
    ASSERT(ptr, "%s size:%d", __func__, size);
    memset(ptr, 0, size);
    int_unlock(lock);
    return ptr;
}

static GAF_ENCODER_CC_MEDIA_DATA_T *gaf_encoder_cc_data_frame_malloc(uint32_t packet_len)
{
    GAF_ENCODER_CC_MEDIA_DATA_T *encoder_frame_p = NULL;
    uint8_t *buffer = NULL;

    if (packet_len)
    {
        buffer = (uint8_t *)gaf_encoder_cc_heap_malloc(packet_len);
    }
    encoder_frame_p = (GAF_ENCODER_CC_MEDIA_DATA_T *)gaf_encoder_cc_heap_malloc(sizeof(GAF_ENCODER_CC_MEDIA_DATA_T));
    encoder_frame_p->org_addr = (uint8_t *)encoder_frame_p;
    encoder_frame_p->data     = buffer;
    encoder_frame_p->data_len = packet_len;
    return encoder_frame_p;
}

static bool gaf_encoder_cc_list_append(gaf_encoder_cc_buff_list_t *list_info, void *data)
{
    gaf_encoder_cc_buffer_mutex_lock(list_info->encode_mutex);
    bool nRet = list_append(list_info->encode_list, data);
    gaf_encoder_cc_buffer_mutex_unlock(list_info->encode_mutex);
    return nRet;
}

static uint32_t gaf_encoder_cc_list_length(gaf_encoder_cc_buff_list_t *list_info)
{
    gaf_encoder_cc_buffer_mutex_lock(list_info->encode_mutex);
    uint32_t length = list_length(list_info->encode_list);
    gaf_encoder_cc_buffer_mutex_unlock(list_info->encode_mutex);
    return length;
}

static list_node_t *gaf_encoder_cc_list_begin(gaf_encoder_cc_buff_list_t *list_info)
{
    gaf_encoder_cc_buffer_mutex_lock(list_info->encode_mutex);
    list_node_t *node = list_begin(list_info->encode_list);
    gaf_encoder_cc_buffer_mutex_unlock(list_info->encode_mutex);
    return node;
}

static void *gaf_encoder_cc_list_node(gaf_encoder_cc_buff_list_t *list_info)
{
    gaf_encoder_cc_buffer_mutex_lock(list_info->encode_mutex);
    void *data = list_node(list_info->encode_node);
    gaf_encoder_cc_buffer_mutex_unlock(list_info->encode_mutex);
    return data;
}

static bool gaf_encoder_cc_list_remove(gaf_encoder_cc_buff_list_t *list_info, void *data)
{
    gaf_encoder_cc_buffer_mutex_lock(list_info->encode_mutex);
    bool nRet = list_remove(list_info->encode_list, data);
    gaf_encoder_cc_buffer_mutex_unlock(list_info->encode_mutex);
    return nRet;
}

static void gaf_encoder_cc_list_new(gaf_encoder_cc_buff_list_t *list_info, const osMutexDef_t *mutex_def,
    list_free_cb callback, list_mempool_zmalloc zmalloc, list_mempool_free free)
{
    if (NULL == list_info->encode_mutex)
    {
        list_info->encode_mutex = osMutexCreate(mutex_def);
    }
    ASSERT(list_info->encode_mutex, "%s encoder mutex create failed", __func__);

    gaf_encoder_cc_buffer_mutex_lock(list_info->encode_mutex);
    list_info->encode_list = list_new(callback, zmalloc, free);
    gaf_encoder_cc_buffer_mutex_unlock(list_info->encode_mutex);
    LOG_D("encoder new LIST 0x%x -> 0x%x", (uint32_t)list_info, (uint32_t)list_info->encode_list);
}

static void gaf_encode_m55_init_req_received_handler(uint8_t* ptr, uint16_t len)
{
    if (!gaf_codec_cc_m55_off_mcu_env.isGafStreamingOnGoing)
    {
        // avoid the timing
        memcpy((uint8_t *)&(gaf_codec_cc_m55_off_mcu_env.cachedInitReq), ptr, sizeof(GAF_ENCODER_INIT_REQ_T));
        osSignalSet(gaf_codec_m55_encode_processor_thread_id, GAF_CODEC_M55_THREAD_SIGNAL_START);
    }
}

static void gaf_encode_m55_deinit_req_received_handler(uint8_t* ptr, uint16_t len)
{
    // inform the m55 processor thread
    memcpy((uint8_t *)&(gaf_codec_cc_m55_off_mcu_env.cachedDeinitReq), ptr, sizeof(GAF_AUDIO_M55_DEINIT_T));
    osSignalSet(gaf_codec_m55_encode_processor_thread_id, GAF_CODEC_M55_THREAD_SIGNAL_STOP);
}

static void gaf_ecoder_cc_ack_feed_encoded_data_received_handler(uint8_t* ptr, uint16_t len)
{
    if (!gaf_codec_cc_m55_off_mcu_env.isGafStreamingOnGoing)
    {
        LOG_I("gaf stream has already been stopped when get the encoded ack.:%s", __func__);
        return;
    }

    GAF_ENCODER_CC_MEDIA_DATA_T* pMediaData = (GAF_ENCODER_CC_MEDIA_DATA_T *)ptr;
    // romove the gaf audio media list that has been encoded data
    gaf_encoder_cc_list_remove(&gaf_codec_cc_m55_off_mcu_env.encoded_data_list, (void *)(pMediaData->org_addr));

    // inform the processor thread
    osSignalSet(gaf_codec_m55_encode_processor_thread_id, GAF_CODEC_M55_ENCODER_THREAD_SIGNAL_DATA_RECEIVED);
}

static void gaf_encoder_cc_retrigger_req_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_RETRIGGER_REQ_NO_RSP, ptr, len);
}

static void gaf_encoder_cc_feed_encoded_data_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_FEED_ENCODED_DATA_TO_CORE, ptr, len);
}

static void gaf_encoder_cc_ack_fed_pcm_data_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_ACK_ENCODE_FEED_PCM_DATA_PACKET, ptr, len);
}

static void gaf_encoder_cc_ack_feed_pcm_data_successed_transmit_handler(uint8_t* ptr, uint16_t len)
{
    app_dsp_m55_bridge_send_data_without_waiting_rsp(CROSS_CORE_TASK_CMD_GAF_ACK_FEED_PCM_DATA_SUCCEED, ptr, len);
}

static void gaf_encoder_cc_ack_feed_pcm_data_to_core(GAF_ENCODER_ACK_FEED_PCM_DATA_REQ_T* pReq)
{
    app_dsp_m55_bridge_send_cmd(
                    CROSS_CORE_TASK_CMD_GAF_ACK_ENCODE_FEED_PCM_DATA_PACKET, \
                    (uint8_t*)pReq, \
                    sizeof(GAF_ENCODER_ACK_FEED_PCM_DATA_REQ_T));
}

static void gaf_encoder_cc_ack_feed_pcm_data_to_core_m55_open(GAF_ENCODER_ACK_FEED_PCM_DATA_SUCC_REQ_T* pReq)
{
    app_dsp_m55_bridge_send_cmd(
                    CROSS_CORE_TASK_CMD_GAF_ACK_FEED_PCM_DATA_SUCCEED, \
                    (uint8_t *)pReq, \
                    sizeof(GAF_ENCODER_ACK_FEED_PCM_DATA_SUCC_REQ_T));
}

static void gaf_encoder_cc_feed_encoded_data_to_core(GAF_ENCODER_CC_MEDIA_DATA_T* pEncodedata)
{
    gaf_encoder_m55_accumulate_cmd_send(
                    CROSS_CORE_TASK_CMD_GAF_FEED_ENCODED_DATA_TO_CORE, \
                    (uint8_t*)pEncodedata);
}

#ifdef TX_ALGO_UPSAMPLING_WORKAROUND
static inline int32_t POSSIBLY_UNUSED _upsampling_by_2_filter(int32_t PcmValue)
{
    // butter(4, 6000/16000,'low');
    const float NUM[5] = {0.037973, 0.151892, 0.227838, 0.151892, 0.037973};
    const float DEN[5] = {1.000000, -0.978369, 0.790086, -0.241882, 0.037734};

    // FIXME: Need to reset it when call restart
    static float y0 = 0, y1 = 0, y2 = 0,y3=0,y4=0, x0 = 0, x1 = 0, x2 = 0,x3=0, x4=0;
    int32_t PcmOut = 0;

    x0 = PcmValue;
    y0 = x0*NUM[0] + x1*NUM[1] + x2*NUM[2] + x3*NUM[3] + x4*NUM[4] - y1*DEN[1] - y2*DEN[2] - y3*DEN[3] - y4*DEN[4];
    y4 = y3;
    y3 = y2;
    y2 = y1;
    y1 = y0;
    x4 = x3;
    x3 = x2;
    x2 = x1;
    x1 = x0;

    PcmOut = (int32_t)y0;

    PcmOut = __SSAT(PcmOut, 24);

    return (int32_t)PcmOut;
}
#endif

static void gaf_cc_speech_tx_algo_init(GAF_ENCODER_INIT_REQ_T* pReq)
{
#ifdef SCO_DSP_ACCEL
    LOG_I("[LE Call] bypass:%d,frame_len:%d,sample_rate:%d,ch_num:%d,bits:%d,anc_mode:%d",
        pReq->tx_algo_cfg.bypass,
        pReq->tx_algo_cfg.frame_len,
        pReq->tx_algo_cfg.sample_rate,
        pReq->tx_algo_cfg.channel_num,
        pReq->tx_algo_cfg.bits,
        pReq->tx_algo_cfg.anc_mode);

    if ((pReq->tx_algo_cfg.bypass == false) &&
#ifndef GAF_TX_ALGO_RUN_ON_SPEECH_THREAD
        (pReq->tx_algo_cfg.frame_len == 160) &&
#endif
        (pReq->tx_algo_cfg.channel_num == PCM_CHANNEL_NUM) &&
        ((pReq->tx_algo_cfg.sample_rate == 16000) || (pReq->tx_algo_cfg.sample_rate == 32000)) &&
        (pReq->tx_algo_cfg.bits == 24)) {
        g_tx_algo_enabled = true;
    } else {
        g_tx_algo_enabled = false;
    }

    LOG_I("[LE Call] g_tx_algo_enabled: %d", g_tx_algo_enabled);

    if (g_tx_algo_enabled == true) {
#ifdef GAF_TX_ALGO_RUN_ON_SPEECH_THREAD
        SCO_DSP_CFG_T dsp_cfg;
        memset(&dsp_cfg, 0, sizeof(SCO_DSP_CFG_T));
        dsp_cfg.sample_rate = pReq->tx_algo_cfg.sample_rate;
        dsp_cfg.frame_len   = SPEECH_PROCESS_FRAME_LEN_MAX;
        dsp_cfg.mic_num     = PCM_CHANNEL_MIC_NUM;
        dsp_cfg.mode        = true;
        dsp_cfg.delay       = SPEECH_PROCESS_FRAME_LEN_MAX * 2;
        dsp_cfg.anc_mode    = 0;
        dsp_cfg.single_core = true;
        speech_algo_dsp_rpc_init(&dsp_cfg);
#else
        speech_algo_dsp_cfg_t speech_algo_dsp_cfg;
        speech_algo_dsp_cfg.sample_rate = pReq->tx_algo_cfg.sample_rate;
        speech_algo_dsp_cfg.frame_len = SPEECH_PROCESS_FRAME_LEN_MAX;
        speech_algo_dsp_cfg.mic_num = 3;
        speech_algo_dsp_cfg.mode = 1;
        speech_algo_dsp_cfg.delay = 0;
        speech_algo_dsp_cfg.anc_mode = pReq->tx_algo_cfg.anc_mode;
        speech_algo_dsp_cfg.rxbwe_mode = 0;
        if (!gaf_speech_heap_buf) {
            gaf_speech_heap_buf = (uint8_t *)off_bth_syspool_calloc(SV_MEM_POOL_BUF_SIZE, sizeof(uint8_t));
        }
        speech_algo_heap_init(gaf_speech_heap_buf, SV_MEM_POOL_BUF_SIZE);
        speech_algo_open(&speech_algo_dsp_cfg);
#endif

        if ((uint32_t)pReq->stream_cfg.sample_rate != pReq->tx_algo_cfg.sample_rate) {
            ASSERT(pReq->stream_cfg.sample_rate % pReq->tx_algo_cfg.sample_rate == 0, "[%s] %d, %d. Can not do resample", __func__, pReq->stream_cfg.sample_rate, pReq->tx_algo_cfg.sample_rate);
            g_tx_algo_upsampling_factor = pReq->stream_cfg.sample_rate / pReq->tx_algo_cfg.sample_rate;
            ASSERT(pReq->tx_algo_cfg.channel_num >= g_tx_algo_upsampling_factor, "[%s] ch_num: %d, factor: %d",  __func__, pReq->tx_algo_cfg.channel_num, g_tx_algo_upsampling_factor);
            TRACE(4, "[%s] Resample %d--> %d. factor: %d", __func__, pReq->tx_algo_cfg.sample_rate, pReq->stream_cfg.sample_rate, g_tx_algo_upsampling_factor);
#ifdef TX_ALGO_UPSAMPLING_WORKAROUND
            ASSERT(pReq->stream_cfg.sample_rate == 32000, "[%s] %d, %d. Can not do resample", __func__, pReq->stream_cfg.sample_rate, pReq->tx_algo_cfg.sample_rate);
            g_tx_algo_upsampling_enabled = true;
#else
            g_tx_algo_upsampling_st = multi_iir_resample_init(pReq->tx_algo_cfg.frame_len,
                                                            pReq->tx_algo_cfg.bits,
                                                            1,
                                                            iir_resample_choose_mode(pReq->tx_algo_cfg.sample_rate, pReq->stream_cfg.sample_rate));
#endif
        } else {
#ifdef TX_ALGO_UPSAMPLING_WORKAROUND
            g_tx_algo_upsampling_enabled = false;
#else
            g_tx_algo_upsampling_st = NULL;
#endif
            g_tx_algo_upsampling_factor = 1;
        }

        g_tx_algo_in_pcm_len = 0;
        if (g_tx_algo_out_pcm_buf_ptr == NULL) {
            g_tx_algo_out_pcm_buf_ptr = (int32_t *)off_bth_syspool_calloc(SPEECH_PROCESS_FRAME_LEN_MAX, sizeof(int32_t));
#ifdef GAF_TX_ALGO_RUN_ON_SPEECH_THREAD
            g_tx_algo_input_pcm_buf_ptr[PCM_CHANNEL_MIC1] = (int32_t *)off_bth_syspool_calloc(pReq->tx_algo_cfg.frame_len * PCM_CHANNEL_MIC_NUM, sizeof(int32_t));
            for (uint32_t ch = 1; ch < PCM_CHANNEL_NUM; ch++) {
                g_tx_algo_input_pcm_buf_ptr[ch] = NULL;
            }
            for (uint32_t ch = PCM_CHANNEL_MIC_NUM; ch < PCM_CHANNEL_NUM; ch++) {
                g_tx_algo_input_pcm_buf_ptr[ch] = (int32_t *)off_bth_syspool_calloc(pReq->tx_algo_cfg.frame_len, sizeof(int32_t));
            }
#else
            for (uint32_t ch = 0; ch < PCM_CHANNEL_NUM; ch++) {
                g_tx_algo_input_pcm_buf_ptr[ch] = (int32_t *)off_bth_syspool_calloc(SPEECH_PROCESS_FRAME_LEN_MAX, sizeof(int32_t));
            }
#endif
        }
    }

#ifdef CALC_MIPS_CNT_THD_MS
    uint32_t sysfreq = 196; // app_sysfreq_get_value(APP_SYSFREQ_208M);
    uint32_t tx_algo_frame_us = 20 * 1000;
    uint32_t encoder_frame_us = pReq->stream_cfg.frame_dms * 100;
    calc_algo_mips_reset(CALC_ALGO_MIPS_USER_TX, tx_algo_frame_us, CALC_MIPS_CNT_THD_MS * 1000 / tx_algo_frame_us, "TX");
    calc_algo_mips_reset(CALC_ALGO_MIPS_USER_ENCODER, encoder_frame_us, CALC_MIPS_CNT_THD_MS * 1000 / encoder_frame_us, "ENCODER");

    calc_algo_set_cpu_freq_MHz(sysfreq);

    TRACE(5, "[%s] sysfreq: %d, tx algo frame us=%d, encoder frame us=%d", __func__, \
        sysfreq, tx_algo_frame_us, encoder_frame_us);
#endif

#ifdef GAF_TX_ALGO_UPSAMPLING_DUMP
    data_dump_init();
#endif
#endif
}

static void gaf_cc_speech_tx_algo_deinit(void)
{
#ifdef SCO_DSP_ACCEL
    if (g_tx_algo_enabled == true) {
#ifdef TX_ALGO_UPSAMPLING_WORKAROUND
        if (g_tx_algo_upsampling_enabled) {
            g_tx_algo_upsampling_enabled = false;
            g_tx_algo_upsampling_factor = 1;
        }
#else
        if (g_tx_algo_upsampling_st) {
            iir_resample_destroy(g_tx_algo_upsampling_st);
            g_tx_algo_upsampling_st = NULL;
            g_tx_algo_upsampling_factor = 1;
        }
#endif

#ifdef GAF_TX_ALGO_RUN_ON_SPEECH_THREAD
        speech_algo_dsp_rpc_deinit();
#else
        speech_algo_close();
        speech_algo_heap_deinit();
#endif
        g_tx_algo_enabled = false;
    }
#endif
}

static void gaf_cc_speech_tx_algo_process(GAF_ENCODER_CC_MEDIA_DATA_T *pMediaPacket)
{
#ifdef SCO_DSP_ACCEL
    if (gaf_codec_cc_m55_off_mcu_env.cachedInitReq.tx_algo_cfg.channel_num == 0) {
        return;
    }

    int32_t *pcm_buf = (int32_t *)pMediaPacket->data;
    uint32_t pcm_len = pMediaPacket->data_len / sizeof(int32_t);
    uint32_t channel_num = gaf_codec_cc_m55_off_mcu_env.cachedInitReq.tx_algo_cfg.channel_num;
    uint32_t frame_len = pcm_len / channel_num;

    if (g_tx_algo_enabled == false) {
        for (uint32_t i = 0; i < frame_len; i++) {
            pcm_buf[i] = pcm_buf[channel_num * i];
        }
        return;
    }

    ASSERT(channel_num == PCM_CHANNEL_NUM, "[%s] Invalid ch num: %d != %d", __func__, channel_num, PCM_CHANNEL_NUM);
    LOG_D("[%s] pcm_len: %d, frame_len: %d, channel_num: %d", __func__, pcm_len, frame_len, channel_num);

#ifdef GAF_TX_ALGO_RUN_ON_SPEECH_THREAD
    for (uint32_t ch = 0; ch < PCM_CHANNEL_MIC_NUM; ch++) {
        for (uint32_t i=0; i<frame_len; i++) {
            g_tx_algo_input_pcm_buf_ptr[PCM_CHANNEL_MIC1][PCM_CHANNEL_MIC_NUM * i + ch] = pcm_buf[PCM_CHANNEL_NUM * i + ch];
        }
    }

    for (uint32_t ch = PCM_CHANNEL_MIC_NUM; ch < PCM_CHANNEL_NUM; ch++) {
        for (uint32_t i=0; i<frame_len; i++) {
            g_tx_algo_input_pcm_buf_ptr[ch][i] = pcm_buf[PCM_CHANNEL_NUM * i + ch];
        }
    }

    SCO_DSP_TX_PCM_T pcm_cfg;
    memset(&pcm_cfg, 0, sizeof(pcm_cfg));
    pcm_cfg.mic = g_tx_algo_input_pcm_buf_ptr[PCM_CHANNEL_MIC1];
    pcm_cfg.ref = g_tx_algo_input_pcm_buf_ptr[PCM_CHANNEL_REF];
    pcm_cfg.vpu = g_tx_algo_input_pcm_buf_ptr[PCM_CHANNEL_VPU];
    pcm_cfg.out = pcm_buf;
    pcm_cfg.frame_len = frame_len;
    pcm_cfg.anc_mode = 0;

    speech_algo_dsp_rpc_capture_process(&pcm_cfg);

    // SV algo just process 16 bit, convert 16bit to 24bit
    for (uint32_t i=0; i<frame_len; i++) {
        pcm_buf[i] = __SSAT(pcm_buf[i] << 8, 24);
    }
#else
    g_tx_algo_in_pcm_len += frame_len;
    // TRACE(3, "[%s] frame_len: %d, process_pcm_len: %d", __func__, frame_len, g_tx_algo_in_pcm_len);
    if (g_tx_algo_in_pcm_len == SPEECH_PROCESS_FRAME_LEN_MAX) {
        // TRACE(2, "[%s] Line: %d", __func__, __LINE__);
        for (uint32_t ch = 0; ch < channel_num; ch++) {
            for (uint32_t i=0; i<frame_len; i++) {
                g_tx_algo_input_pcm_buf_ptr[ch][frame_len + i] = pcm_buf[channel_num * i + ch];
            }
        }

#ifdef CALC_MIPS_CNT_THD_MS
        uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

        speech_algo_capture_process(g_tx_algo_input_pcm_buf_ptr, g_tx_algo_out_pcm_buf_ptr, SPEECH_PROCESS_FRAME_LEN_MAX);

#ifdef CALC_MIPS_CNT_THD_MS
        uint32_t end_ticks = hal_fast_sys_timer_get();
        calc_algo_mips(CALC_ALGO_MIPS_USER_TX, start_ticks, end_ticks);
#endif

        // SV algo just process 16 bit, convert 16bit to 24bit
        for (uint32_t i=0; i<frame_len; i++) {
            pcm_buf[i] = __SSAT(g_tx_algo_out_pcm_buf_ptr[i] << 8, 24);
        }

        g_tx_algo_in_pcm_len = 0;
    } else {
        // TRACE(2, "[%s] Line: %d", __func__, __LINE__);
        for (uint32_t ch = 0; ch < channel_num; ch++) {
            for (uint32_t i=0; i<frame_len; i++) {
                g_tx_algo_input_pcm_buf_ptr[ch][i] = pcm_buf[channel_num * i + ch];
            }
        }

        // SV algo just process 16 bit, convert 16bit to 24bit
        for (uint32_t i=0; i<frame_len; i++) {
            pcm_buf[i] = __SSAT(g_tx_algo_out_pcm_buf_ptr[frame_len + i] << 8, 24);
        }
    }
#endif

    pMediaPacket->data_len = frame_len * sizeof(int32_t);

#ifdef GAF_TX_ALGO_UPSAMPLING_DUMP
    data_dump_run(0, "tx_algo_16k", pMediaPacket->data, pMediaPacket->data_len);
#endif

#ifdef TX_ALGO_UPSAMPLING_WORKAROUND
    if (g_tx_algo_upsampling_enabled) {
        // interp by 2
        for (int32_t i = frame_len - 1; i >= 0; i--) {
            for (uint32_t j = 0; j < g_tx_algo_upsampling_factor; j++) {
                    pcm_buf[g_tx_algo_upsampling_factor * i + j] = pcm_buf[i];
            }
        }
        // Low pass filter
        for (uint32_t i=0; i < frame_len * g_tx_algo_upsampling_factor; i++) {
            pcm_buf[i] = _upsampling_by_2_filter(pcm_buf[i]);
        }
        pMediaPacket->data_len *= g_tx_algo_upsampling_factor;
    }
#else
    if (g_tx_algo_upsampling_st) {
        iir_resample_process(g_tx_algo_upsampling_st, pcm_buf, pcm_buf, frame_len);
        pMediaPacket->data_len *= g_tx_algo_upsampling_factor;
    }
#endif // TX_ALGO_UPSAMPLING_WORKAROUND

#ifdef GAF_TX_ALGO_UPSAMPLING_DUMP
    data_dump_run(1, "tx_algo_32k", pMediaPacket->data, pMediaPacket->data_len);
#endif
#endif
}

static void gaf_encoder_cc_process_encoding(GAF_ENCODER_CC_MEDIA_DATA_T* pMediaPacket)
{
    GAF_ENCODER_CC_MEDIA_DATA_T* p_stored_encoded_data_info =
        gaf_encoder_cc_data_frame_malloc(gaf_codec_cc_m55_off_mcu_env.maxEncodedDataSizePerFrame);
    p_stored_encoded_data_info->time_stamp = pMediaPacket->time_stamp;
    uint16_t pcmdata_len = pMediaPacket->data_len;
    uint8_t  *pcmdata    = pMediaPacket->data;
    uint16_t frame_size  = pMediaPacket->frame_size * gaf_codec_cc_m55_off_mcu_env.channels;

#if defined(CALC_MIPS_CNT_THD_MS) && defined(SCO_DSP_ACCEL)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

    // uint32_t lock = int_lock();
    gaf_codec_cc_m55_off_mcu_env.gaf_audio_encoder.gaf_audio_encoder_cc_pcm_frame(pcmdata_len, pcmdata, frame_size, (uint8_t*)p_stored_encoded_data_info);
    // int_unlock(lock);

#if defined(CALC_MIPS_CNT_THD_MS) && defined(SCO_DSP_ACCEL)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    calc_algo_mips(CALC_ALGO_MIPS_USER_ENCODER, start_ticks, end_ticks);
#endif

    gaf_encoder_cc_list_append(&gaf_codec_cc_m55_off_mcu_env.encoded_data_list, p_stored_encoded_data_info);

    // send encoding data to mcu core

    gaf_encoder_cc_feed_encoded_data_to_core(p_stored_encoded_data_info);
}

static void gaf_encoder_cc_process_pcm_data(void)
{
    gaf_encoder_cc_buff_list_t *list = &gaf_codec_cc_m55_off_mcu_env.pcm_data_list;
    GAF_ENCODER_CC_MEDIA_DATA_T* decoded_frame_p = NULL;

    uint32_t currentencodedListLen = gaf_encoder_cc_list_length(
        &gaf_codec_cc_m55_off_mcu_env.encoded_data_list);

    if (currentencodedListLen >= GAF_ENCODER_CC_MAX_PCM_DATA_CNT_LIMITER)
    {
        return;
    }

    do
    {
        list->encode_node = gaf_encoder_cc_list_begin(list);
        if (NULL == list->encode_node)
        {
            LOG_D("gaf_m55_encoder_pcm_data_list is empty");
            break;
        }

        decoded_frame_p = (GAF_ENCODER_CC_MEDIA_DATA_T *)gaf_encoder_cc_list_node(list);

        if (gaf_codec_cc_m55_off_mcu_env.isGafStreamingOnGoing)
        {
            gaf_codec_cc_m55_off_mcu_env.isEncodingJustStarted = false;
        }
        gaf_cc_speech_tx_algo_process(decoded_frame_p);
        gaf_encoder_cc_process_encoding(decoded_frame_p);
        gaf_encoder_cc_list_remove(list, decoded_frame_p);
    } while (0);

    if (gaf_encoder_cc_list_length(list) > 0)
    {
        osSignalSet(gaf_codec_m55_encode_processor_thread_id, GAF_CODEC_M55_ENCODER_THREAD_SIGNAL_DATA_RECEIVED);
    }
}

static void gaf_encoder_cc_pcm_data_received_handler(uint8_t* param, uint16_t len)
{
    GAF_ENCODER_CC_MEDIA_DATA_T* p_pcm_media_data_info;
    bool isGoing = true;
    bool isInformPacketReceived = false;

    // acculate the current length
    uint16_t unitofInfo  = sizeof(GAF_ENCODER_CC_MEDIA_DATA_T);
    uint8_t packetCnt = len / unitofInfo;

    while(packetCnt--)
    {
        p_pcm_media_data_info = (GAF_ENCODER_CC_MEDIA_DATA_T*)param;
        isGoing = gaf_codec_cc_m55_off_mcu_env.isGafStreamingOnGoing;
        if (!isGoing)
        {
            LOG_I("m55 gaf stream encoder has already been stopped.:%s", __func__);
            // send notification to bth
            GAF_ENCODER_ACK_FEED_PCM_DATA_REQ_T ackReq;
            memcpy((uint8_t *)&(ackReq.ackedDataPacket), (uint8_t *)p_pcm_media_data_info, sizeof(GAF_ENCODER_CC_MEDIA_DATA_T));
            ackReq.encodedPacketEntryCount = 0;
            ackReq.pcmPacketEntryCount     = 0;
            gaf_encoder_cc_ack_feed_pcm_data_to_core(&ackReq);
            return;
        }
        else
        {
            GAF_ENCODER_CC_MEDIA_DATA_T* p_stored_pcm_media_data_info =
                gaf_encoder_cc_data_frame_malloc(p_pcm_media_data_info->data_len);

            p_stored_pcm_media_data_info->time_stamp = p_pcm_media_data_info->time_stamp;
            p_stored_pcm_media_data_info->coorespondingPcmDataPacketPtr = (void*)p_pcm_media_data_info->org_addr;

            // copy to local buffer for encoding
            memcpy(p_stored_pcm_media_data_info->data, p_pcm_media_data_info->data,
                p_pcm_media_data_info->data_len);

            p_stored_pcm_media_data_info->frame_size = p_pcm_media_data_info->frame_size;

            gaf_encoder_cc_list_append(&gaf_codec_cc_m55_off_mcu_env.pcm_data_list,
                p_stored_pcm_media_data_info);

            if (!isInformPacketReceived)
            {
                // send notification to mcu core based on the m55 thread signal
                if (gaf_encoder_cc_list_length(&gaf_codec_cc_m55_off_mcu_env.encoded_data_list)
                    < GAF_ENCODER_CC_MAX_PCM_DATA_CNT_LIMITER)
                {
                    // ensure the next frame will not interrupt the current frame
                    isInformPacketReceived = true;

                    // inform bth to delete the pcm data pointer
                    GAF_ENCODER_ACK_FEED_PCM_DATA_SUCC_REQ_T ackPcmDataReq;
                    ackPcmDataReq.coorespondingPcmDataPacketPtr =
                                p_stored_pcm_media_data_info->coorespondingPcmDataPacketPtr;
                    ackPcmDataReq.is_successed = true;
                    gaf_encoder_cc_ack_feed_pcm_data_to_core_m55_open(&ackPcmDataReq);

                    // inform the processor thread
                    osSignalSet(gaf_codec_m55_encode_processor_thread_id, GAF_CODEC_M55_ENCODER_THREAD_SIGNAL_DATA_RECEIVED);
                }
            }
        }
        param += unitofInfo;
    }
}


static void gaf_encoder_cc_m55_stream_reset(void)
{
    gaf_codec_cc_m55_off_mcu_env.isEncodingJustStarted = true;
    gaf_codec_cc_m55_off_mcu_env.lastEncodedSeqNum     = 0;
    gaf_codec_cc_m55_off_mcu_env.lastEncodedSubSeqNum  = 0;

    InitCQueue(&gaf_codec_cc_m55_off_mcu_env.toBeAddedDecodedPacketQueue,
        sizeof(gaf_encoder_cached_to_be_removed_frame_ptr_queue_buf),
        (CQItemType *)gaf_encoder_cached_to_be_removed_frame_ptr_queue_buf);
}

static void gaf_encoder_cc_m55_init_handler(void)
{
    bool clearMem = false;
    LOG_I("%s start", __func__);

    app_sysfreq_req(APP_SYSFREQ_USER_AOB_CAPTURE, APP_SYSFREQ_208M);

    gaf_encoder_cc_m55_stream_reset();

    GAF_ENCODER_INIT_REQ_T* pReq = &(gaf_codec_cc_m55_off_mcu_env.cachedInitReq);
    gaf_codec_cc_m55_off_mcu_env.channels = pReq->stream_cfg.channels;
    gaf_codec_cc_m55_off_mcu_env.isGafStreamingOnGoing = true;
    gaf_codec_cc_m55_off_mcu_env.maxEncodedDataSizePerFrame = pReq->maxEncodedDataSizePerFrame;

    if (NULL == gaf_encoder_cc_mem_pool)
    {
        off_bth_syspool_get_buff(&gaf_encoder_cc_mem_pool, GAF_ENCODER_CC_MEM_POOL_SIZE);
        ASSERT(gaf_encoder_cc_mem_pool, "%s size:%d", __func__, GAF_ENCODER_CC_MEM_POOL_SIZE);
        clearMem = true;
    }

    gaf_encoder_cc_m55_heap_init(gaf_encoder_cc_mem_pool, GAF_ENCODER_CC_MEM_POOL_SIZE);

    gaf_encoder_cc_list_new(&gaf_codec_cc_m55_off_mcu_env.encoded_data_list,
                (osMutex(gaf_encoder_cc_encoded_data_buff_mutex)),
                gaf_encoder_cc_data_free,
                gaf_encoder_cc_heap_cmalloc,
                gaf_encoder_cc_heap_free);

    gaf_encoder_cc_list_new(&gaf_codec_cc_m55_off_mcu_env.pcm_data_list,
                (osMutex(gaf_encoder_cc_pcm_data_buff_mutex)),
                gaf_encoder_cc_data_free,
                gaf_encoder_cc_heap_cmalloc,
                gaf_encoder_cc_heap_free);

    GAF_AUDIO_CC_ENCODER_T *p_gaf_audio_cc_encoder = NULL;

    p_gaf_audio_cc_encoder = &gaf_audio_cc_lc3_encoder_config;

    // adjust m55 encode frequency to reduce the power consumation
    enum APP_SYSFREQ_FREQ_T gaf_encoder_m55_freq = gaf_encoder_cc_base_freq;

    gaf_encoder_cc_base_freq = gaf_audio_stream_encoder_adjust_m55_freq(pReq->stream_cfg.frame_size,
        pReq->stream_cfg.channels, pReq->stream_cfg.frame_dms,
        pReq->stream_cfg.sample_rate, gaf_encoder_m55_freq,
        pReq->stream_playback_state, pReq->stream_capture_state);

    memcpy((uint8_t *)&p_gaf_audio_cc_encoder->stream_config ,
            (uint8_t *)&pReq->stream_cfg,
            sizeof(GAF_AUDIO_CC_ENCODE_INPUT_T));

    memcpy((uint8_t *)&gaf_codec_cc_m55_off_mcu_env.gaf_audio_encoder,
            (uint8_t *)p_gaf_audio_cc_encoder,
            sizeof(GAF_AUDIO_CC_ENCODER_T));

    if (clearMem) {
        gaf_codec_cc_m55_off_mcu_env.gaf_audio_encoder.gaf_audio_encoder_cc_clear_buffer_pointer();
    }

    gaf_codec_cc_m55_off_mcu_env.gaf_audio_encoder.gaf_audio_encoder_cc_init();

    gaf_cc_speech_tx_algo_init(pReq);

    GAF_AUDIO_ENCODER_INIT_RSP_T rsp;
    app_dsp_m55_bridge_send_rsp(CROSS_CORE_TASK_CMD_GAF_ENCODE_INIT_WAITING_RSP,
        (uint8_t *)&rsp, sizeof(rsp));

#if defined(SCO_DSP_ACCEL) && !defined(GAF_TX_ALGO_RUN_ON_SPEECH_THREAD)
    if (g_tx_algo_enabled == true) {
        app_sysfreq_req(APP_SYSFREQ_USER_AOB_CAPTURE, APP_SYSFREQ_144M);
    } else
#endif
    {
        app_sysfreq_req(APP_SYSFREQ_USER_AOB_CAPTURE, gaf_encoder_cc_base_freq);
    }
}

static void gaf_encoder_cc_off_bth_syspool_cb(void)
{
    gaf_encoder_cc_mem_pool = NULL;
#ifdef SCO_DSP_ACCEL
    gaf_speech_heap_buf = NULL;
    g_tx_algo_out_pcm_buf_ptr = NULL;
    for (uint32_t ch = 0; ch < PCM_CHANNEL_NUM; ch++) {
        g_tx_algo_input_pcm_buf_ptr[ch] = NULL;
    }
#endif
}

static void gaf_codec_m55_encode_processor_thread(const void *arg)
{
    while(1)
    {
        osEvent evt;
        // wait any signal
        evt = osSignalWait(0x0, osWaitForever);

        // get role from signal value
        if (osEventSignal == evt.status)
        {
            // get start signal to init encoder
            if (evt.value.signals & GAF_CODEC_M55_THREAD_SIGNAL_START)
            {
                off_bth_syspool_init(SYSPOOL_USER_GAF_ENCODE, gaf_encoder_cc_off_bth_syspool_cb);
                gaf_encoder_cc_m55_init_handler();
            }

            // get stop signal to deinit encoder
            if (evt.value.signals & GAF_CODEC_M55_THREAD_SIGNAL_STOP)
            {
                if (gaf_codec_cc_m55_off_mcu_env.isGafStreamingOnGoing)
                {
                    gaf_cc_speech_tx_algo_deinit();

                    gaf_codec_cc_m55_off_mcu_env.gaf_audio_encoder.gaf_audio_encoder_cc_deinit();

                    gaf_encoder_cc_m55_stream_reset();

                    gaf_codec_cc_m55_off_mcu_env.isGafStreamingOnGoing = false;

                    app_sysfreq_req(APP_SYSFREQ_USER_AOB_CAPTURE, APP_SYSFREQ_32K);

                    off_bth_syspool_deinit(SYSPOOL_USER_GAF_ENCODE);
                }

                /// feed encoder deinit signal to bth after m55 finished deinit task
                GAF_AUDIO_M55_DEINIT_T p_encoder_deinit_rsp;
                p_encoder_deinit_rsp.is_bis          = gaf_codec_cc_m55_off_mcu_env.cachedDeinitReq.is_bis;
                p_encoder_deinit_rsp.is_bis_src      = gaf_codec_cc_m55_off_mcu_env.cachedDeinitReq.is_bis_src;
                p_encoder_deinit_rsp.is_mobile_role  = gaf_codec_cc_m55_off_mcu_env.cachedDeinitReq.is_mobile_role;
                p_encoder_deinit_rsp.capture_deinit  = gaf_codec_cc_m55_off_mcu_env.cachedDeinitReq.capture_deinit;
                p_encoder_deinit_rsp.playback_deinit = gaf_codec_cc_m55_off_mcu_env.cachedDeinitReq.playback_deinit;
                p_encoder_deinit_rsp.con_lid         = gaf_codec_cc_m55_off_mcu_env.cachedDeinitReq.con_lid;
                p_encoder_deinit_rsp.context_type    = gaf_codec_cc_m55_off_mcu_env.cachedDeinitReq.context_type;

                app_dsp_m55_bridge_send_rsp(CROSS_CORE_TASK_CMD_GAF_ENCODE_DEINIT_WAITING_RSP, \
                                            (uint8_t *)&(p_encoder_deinit_rsp), \
                                            sizeof(GAF_AUDIO_M55_DEINIT_T));
            }

            if (evt.value.signals & GAF_CODEC_M55_ENCODER_THREAD_SIGNAL_DATA_RECEIVED)
            {
                if (gaf_codec_cc_m55_off_mcu_env.isGafStreamingOnGoing)
                {
                    gaf_encoder_cc_process_pcm_data();
                }
            }
        }
    }
}

void gaf_codec_encoder_m55_init(void)
{
    if (NULL == gaf_codec_m55_encode_processor_thread_id)
    {
        gaf_codec_m55_encode_processor_thread_id =
        osThreadCreate(osThread(gaf_codec_m55_encode_processor_thread), NULL);
    }
}
#endif
