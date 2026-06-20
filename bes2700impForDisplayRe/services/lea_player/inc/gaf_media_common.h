/**
 * @file gaf_media_common.h
 * @author BES AI team
 * @version 0.1
 * @date 2020-08-31
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


#ifndef __GAF_MEDIA_COMMON_H__
#define __GAF_MEDIA_COMMON_H__

/*****************************header include********************************/
#include "list.h"
#include "cmsis_os.h"
#include "gaf_media_pid.h"
#include "bluetooth_bt_api.h"

/******************************macro defination*****************************/
#define GAF_MAXIMUM_CONNECTION_COUNT                        (3)
#define GAF_INVALID_ASE_INDEX                               (0xFF)
#define GAF_INVALID_CONNECTION_IDX                          (0xFF)
#define GAF_AUDIO_DFT_PLAYBACK_LIST_IDX                     (0)
// Every stream direction per conn that used ase number cnt max
#define GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT                  (2)

#ifdef DYNAMIC_SET_PB_TIME
#define GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER             (40)
#else
#define GAF_AUDIO_MEDIA_DATA_PACKET_NUM_LIMITER             (15)
#endif
#define GAF_AUDIO_MAX_CHANNELS                              (2)
#define GAF_AUDIO_CONTROLLER_2_HOST_LATENCY_US              (2000)
#define GAF_AUDIO_CAPTURE_TRIGGER_GUARD_TIME_US             (2000)
#define GAF_AUDIO_CAPTURE_TRIGGER_DELAY_IN_US               (100000)
#define GAF_AUDIO_DMA_IRQ_HAPPENING_TIME_GAP_TOLERANCE_US   (600)
#define GAF_AUDIO_INVALID_ISO_CHANNEL                       (0xFF)
#define GAF_AUDIO_INVALID_ISO_HANDLE                        (0xFFFF)
#define GAF_AUDIO_INVALID_SEQ_NUMBER                        (0xFFFF)
#define GAF_AUDIO_ALPHA_PRAMS_1                             (3)
#define GAF_AUDIO_ALPHA_PRAMS_2                             (4)
// Max allowed different bt time (2000us)
#define GAF_AUDIO_MAX_DIFF_BT_TIME                          (2000)
// Min need time to set dma (7000us)
#define GAF_AUDIO_DMA_SETUP_MIN_TIME                        (7000)

#ifdef AOB_MOBILE_ENABLED
#define GAF_AUDIO_ASE_TOTAL_COUNT                           (4 * BLE_AUDIO_CONNECTION_CNT)
#define GAF_AUDIO_FIXED_MOBILE_SIDE_PRESENT_LATENCY_US      (30000)
#else
#define GAF_AUDIO_ASE_TOTAL_COUNT                           (4)
#endif

#if defined(BLE_USB_AUDIO_SUPPORT) || defined(AOB_LOW_LATENCY_MODE)
#define GAF_MARGIN_BETWEEN_TRIGGER_TIME_AND_CURRENT_TIME_US 5000
#else
#define GAF_MARGIN_BETWEEN_TRIGGER_TIME_AND_CURRENT_TIME_US 10000
#endif

/// Maximum diff value of a 1us Bluetooth clock
#define GAF_AUDIO_MAX_DIFF_TIME                             ((1L<<16) - 1)
#define GAF_AUDIO_CLK_SUB(clock_a, clock_b)                 ((uint32_t)(((clock_a) - (clock_b)) & GAF_AUDIO_MAX_DIFF_TIME))
#define GAF_AUDIO_ABS(value)                                ((value) > 0?(value):(-value))
#define GAF_AUDIO_CLK_DIFF(clock_a, clock_b)                ((GAF_AUDIO_CLK_SUB((clock_b), (clock_a)) > (uint32_t)((GAF_AUDIO_MAX_DIFF_TIME + 1) >> 1)) ? \
                                                             ((int32_t)((-GAF_AUDIO_CLK_SUB((clock_a), (clock_b))))) : ((int32_t)((GAF_AUDIO_CLK_SUB((clock_b), (clock_a))))))

#define GAF_AUDIO_MAX_32_BIT_DIFF_TIME                      (0xFFFFFFFF)
#define GAF_AUDIO_CLK_32_BIT_SUB(clock_a, clock_b)          ((uint64_t)(((clock_a) - (clock_b)) & GAF_AUDIO_MAX_32_BIT_DIFF_TIME))
#define GAF_AUDIO_CLK_32_BIT_DIFF(clock_a, clock_b)         ((GAF_AUDIO_CLK_32_BIT_SUB((clock_b), (clock_a)) > (uint64_t)((GAF_AUDIO_MAX_32_BIT_DIFF_TIME + 1) >> 1)) ? \
                                                             ((int32_t)((-GAF_AUDIO_CLK_32_BIT_SUB((clock_a), (clock_b))))) : ((int32_t)((GAF_AUDIO_CLK_32_BIT_SUB((clock_b), (clock_a))))))


/******************************type defination******************************/

/// CIS Direction
typedef enum
{
    /// Sink direction
    GAF_LOCAL_DIRECTION_SINK = (1<<0),
    /// Source direction
    GAF_LOCAL_DIRECTION_SRC  = (1<<1),
    /// Sink and Source direction
    GAF_LOCAL_DIRECTION_BOTH  = GAF_LOCAL_DIRECTION_SINK|GAF_LOCAL_DIRECTION_SRC,
} GAF_LOCAL_DIRECTION;

typedef enum
{
    GAF_AUDIO_STREAM_CONTEXT_TYPE_MIN = 0,
    GAF_AUDIO_STREAM_CONTEXT_TYPE_MEDIA = GAF_AUDIO_STREAM_CONTEXT_TYPE_MIN,
#ifdef AOB_MOBILE_ENABLED
    GAF_AUDIO_STREAM_CONTEXT_TYPE_CALL,
    GAF_AUDIO_STREAM_CONTEXT_TYPE_AI,
#endif
    // FLEXIBLE type means that both playback and capture streams could be
    // started and stopped in the run-time across the context life span
    GAF_AUDIO_STREAM_CONTEXT_TYPE_FLEXIBLE,

    GAF_AUDIO_STREAM_CONTEXT_TYPE_BIS           = 4,
    GAF_AUDIO_CONTEXT_NUM_MAX
} GAF_AUDIO_STREAM_CONTEXT_TYPE_E;

/// HCI ISO_Data_Load - Packet Status Flag
typedef enum
{
    GAF_STREAM_PLAYBACK = (1<<0),
    GAF_STREAM_CAPTURE  = (1<<1),
} GAF_STREAM_TYPE_E;

typedef enum
{
    GAF_STREAM_PLAYBACK_STATUS_IDLE,
    GAF_STREAM_PLAYBACK_STATUS_BUSY,
} GAF_STREAM_PLAYBACK_STATUS_E;

typedef struct
{
    void *mutex;
    list_t *list;
    list_node_t *node;
} gaf_stream_buff_list_t;

typedef struct
{
    uint32_t sample_rate;
    int32_t num_channels;
    int32_t bits_depth;
    uint32_t frame_samples;
} GAF_AUDIO_INPUT_CONFIG_T;

typedef struct
{
    uint32_t    bypass;
    uint32_t    frame_len;
    uint32_t    sample_rate;
    uint32_t    channel_num;
    uint32_t    bits;
    uint32_t    anc_mode;
} GAF_AUDIO_TX_ALGO_CFG_T;

typedef union
{
    struct
    {
        int lc3_dec_info[24];
        int lc3_enc_info[24];
    } lc3_codec_context;

    // TODO: Add vendor codec info data structure
} gaf_codec_algorithm_context_t;

typedef int (*GAF_AUDIO_START_STREAM_FUNC)(void* pStreamEnv);
typedef void (*GAF_AUDIO_INIT_STREAM_BUF_FUNC)(void* pStreamEnv);
typedef int (*GAF_AUDIO_STOP_STREAM_FUNC)(void* pStreamEnv);
typedef uint32_t (*GAF_AUDIO_STREAM_DMA_IRQ_HANDLER)(uint8_t* ptrData, uint32_t dataLength);
typedef void (*GAF_AUDIO_DEINIT_STREAM_BUF_FUNC)(void* pStreamEnv);

typedef void (*GAF_AUDIO_DECODER_INIT_BUFFER)(void* pStreamEnv, uint8_t alg_context_cnt);
typedef void (*GAF_AUDIO_DECODER_INIT_FUNC)(void* pStreamEnv, uint8_t alg_context_cnt);
typedef void (*GAF_AUDIO_DECODER_DEINIT_FUNC)(void);
typedef int  (*GAF_AUDIO_DECODER_DECODE_FRAME_FUNC)(
    bool isPLC, uint32_t inputDataLength, void* input,
    gaf_codec_algorithm_context_t *algo_context, void* output);
typedef void (*GAF_AUDIO_DECODER_DEINIT_BUFFER)(void* pStreamEnv, uint8_t alg_context_cnt);

typedef void (*GAF_AUDIO_ENCODER_INIT_BUFFER)(void* pStreamEnv);
typedef void (*GAF_AUDIO_ENCODER_INIT_FUNC)(void* pStreamEnv);
typedef void (*GAF_AUDIO_ENCODER_DEINIT_FUNC)(void* pStreamEnv);
typedef void (*GAF_AUDIO_ENCODER_SEND_FRAME_FUNC)(void* pStreamEnv,void *payload, uint32_t payload_size, uint32_t ref_time);
typedef void (*GAF_AUDIO_ENCODER_ENCODE_FRAME_FUNC)(
    void* pStreamEnv, uint32_t timeStampToFill,
    uint32_t inputDataLength, void *input, gaf_codec_algorithm_context_t *algo_context
    , GAF_AUDIO_ENCODER_SEND_FRAME_FUNC cbsend);
typedef void (*GAF_AUDIO_ENCODER_DEINIT_BUFFER)(void* pStreamEnv);

typedef void (*GAF_ADUIO_STREAM_RETRIGGER)(void* pStreamEnv);

/*************************************GAF CUSTOM*****************************************/
/**
 * @brief call after a packet received, input received packet ptr
 * 
 */
typedef uint8_t (*gaf_stream_recv_data_custom_cb)(const gaf_media_data_t* header);

/**
 * @brief call after a packet decoded, input raw data ptr and len
 * 
 */
typedef uint8_t (*gaf_stream_playback_dma_irq_custom_cb)(const gaf_media_data_t* header, 
                                                         const uint8_t* raw_data, uint32_t len);

/**
 * @brief call before sending out an encoded packet, input packet encoded data ptr and len
 * 
 */
typedef uint8_t (*gaf_stream_send_data_custom_cb)(uint8_t* encoded_data, uint32_t len);

/**
 * @brief call before encoding pcm raw data, input raw data and len
 * 
 */
typedef uint8_t (*gaf_stream_capture_dma_irq_custom_cb)(uint8_t* raw_data, uint32_t len);

typedef enum
{
    GAF_AUDIO_UPDATE_STREAM_INFO_TO_START   = 0,
    GAF_AUDIO_UPDATE_STREAM_INFO_TO_STOP    = 1,
} GAF_AUDIO_UPDATE_STREAM_INFO_PURPOSE_E;

typedef enum
{
    GAF_PLAYBACK_STREAM_IDLE                = 0,
    GAF_PLAYBACK_STREAM_WAITING_M55_INIT    = 1,
    GAF_PLAYBACK_STREAM_INITIALIZED         = 2,
    GAF_PLAYBACK_STREAM_START_TRIGGERING    = 3,
    GAF_PLAYBACK_STREAM_STREAMING_TRIGGERED = 4,
} GAF_PLAYBACK_STREAM_STATE_E;

typedef enum
{
    GAF_CAPTURE_STREAM_IDLE                 = 0,
    GAF_CAPTURE_STREAM_INITIALIZED          = 1,
    GAF_CAPTURE_STREAM_START_TRIGGERING     = 2,
    GAF_CAPTURE_STREAM_STREAMING_TRIGGERED  = 3,

} GAF_CAPTURE_STREAM_STATE_E;

typedef enum
{
    // music (playback only) or call (playback + capture)
    GAF_AUDIO_TRIGGER_BY_PLAYBACK_STREAM = 0,

    // AI (capture only)
    GAF_AUDIO_TRIGGER_BY_CAPTURE_STREAM  = 1,

    // Game or Live (peripheral role) or Call (Central role)
    GAF_AUDIO_TRIGGER_BY_ISOLATE_STREAM = 2,

} GAF_AUDIO_TRIGGER_STREAM_TYPE_E;

typedef enum
{
    GAF_AUDIO_STREAM_TYPE_PLAYBACK = (1 << 0),
    GAF_AUDIO_STREAM_TYPE_CAPTURE  = (1 << 1),
    GAF_AUDIO_STREAM_TYPE_FLEXIBLE = (1 << 2),
} GAF_AUDIO_STREAM_TYPE_E;

typedef enum
{
    GAF_STREAM_USER_CASE_MIN,
    GAF_STREAM_USER_CASE_UC_SRV = GAF_STREAM_USER_CASE_MIN,
#ifdef AOB_MOBILE_ENABLED
    GAF_STREAM_USER_CASE_UC_CLI,
#endif
    GAF_STREAM_USER_CASE_MAX,
} GAF_STREAM_USER_CASE_E;

typedef struct
{
    uint16_t    cis_hdl;// CIS handle range 0x100~0x200,BIS handle range 0x200~0x300
    uint8_t     iso_channel_hdl;
    uint32_t    allocation_bf;
} GAF_AUDIO_CIS_STREAM_CHANNEL_INFO_T;
typedef struct
{
    uint16_t    iso_channel_hdl;// CIS handle range 0x100~0x200,BIS handle range 0x200~0x300
    uint16_t    blocks_size;
    uint8_t     select_ch_map;
} GAF_AUDIO_BIS_STREAM_CHANNEL_INFO_T;

typedef struct
{
    uint8_t             playback_buf_depth;
    osTimerId           stream_status_checker_tid;
    osTimerDefEx_t      stream_status_checker_tdef;
    uint8_t             plc_counter;
    uint8_t             playback_irq_counter;
    bool                stable_check_ongoing;
}GAF_AUDIO_DYNC_BUFFER_T;

typedef struct
{
    /*
     * Used to record the info of ASE in this scenario.
     * Currently, it is mainly used on the mobile
     */
    GAF_AUDIO_CIS_STREAM_CHANNEL_INFO_T aseChInfo[GAF_AUDIO_ASE_TOTAL_COUNT];
    GAF_AUDIO_BIS_STREAM_CHANNEL_INFO_T bisChInfo[2];
    uint32_t    sample_rate;
    uint8_t     num_channels;
    uint8_t     bits_depth;
    float       frame_ms;
    uint16_t    encoded_frame_size;
    uint8_t     sdu_num;

    uint8_t*    storedDmaBufPtr;
    uint8_t*    dmaBufPtr;
    uint32_t    dmaChunkSize;
    uint32_t    dmaChunkIntervalUs;

    uint16_t    maxCachedEncodedAudioPacketCount;
    uint32_t    maxEncodedAudioPacketSize;

    uint32_t    presDelayUs;
    uint32_t    cigSyncDelayUs;
    uint32_t    isoIntervalUs;
    uint32_t    bnM2S;
    uint32_t    bnS2M;
    uint8_t     trigger_stream_lid;
    uint8_t*    decode_buf;

#ifdef DYNAMIC_SET_PB_TIME
    GAF_AUDIO_DYNC_BUFFER_T dync_buffer;
#endif
} GAF_AUDIO_STREAM_COMMON_INFO_T;

typedef struct
{
    uint8_t                     cisChannel;
    gaf_stream_buff_list_t      buff_list;
} GAF_STREAM_PLAYBACK_BUFF_T;

#ifdef GAF_CODEC_CROSS_CORE
typedef struct
{
    uint8_t                     cisChannel;
    gaf_stream_buff_list_t      buff_list;
} GAF_STREAM_CAPTURE_BUFF_T;
#endif

typedef struct
{
    // playback stream context
    GAF_PLAYBACK_STREAM_STATE_E playback_stream_state;
    uint16_t                    lastestPlaybackSeqNum[GAF_AUDIO_ASE_TOTAL_COUNT];
    uint16_t                    lastestPlaybackSeqNumR;
    uint16_t                    lastestPlaybackSeqNumL;
    uint32_t                    lastPlaybackDmaIrqTimeUs;
    uint32_t                    playbackTriggerStartTicks;
    uint8_t                     playbackTriggerChannel;

    GAF_STREAM_PLAYBACK_BUFF_T  playback_buff_list[GAF_AUDIO_ASE_TOTAL_COUNT];
#ifdef GAF_CODEC_CROSS_CORE
    uint8_t                     right_cis_channel;
    uint8_t                     left_cis_channel;
    GAF_STREAM_CAPTURE_BUFF_T   m55_capture_buff_list;
#endif
    gaf_media_pid_t             playback_pid_env;
    osTimerId                   playback_trigger_supervisor_timer_id;
    osTimerId                   capture_trigger_supervisor_timer_id;
    bool                        playback_retrigger_onprocess;

    // capture stream context
    GAF_CAPTURE_STREAM_STATE_E  capture_stream_state;
    gaf_stream_buff_list_t      capture_buff_list;
    uint16_t                    latestCaptureSeqNum;
    uint32_t                    lastCaptureDmaIrqTimeUs;
    uint32_t                    captureTriggerStartTicks;
    uint32_t                    lastCaptureDmaIrqTimeUsInTriggerPoint;
    bool                        isUsSinceLatestAnchorPointConfigured;
    int32_t                     usSinceLatestAnchorPoint;
    int32_t                     captureAverageDmaChunkIntervalUs;
    gaf_media_pid_t             capture_pid_env;
    uint8_t                     captureTriggerChannel;
    // trigger time in us = us converted from master_trigger_base_bt_clk_cnt + trigger delay in us
    uint32_t                    master_trigger_base_bt_clk_cnt;

    // when the captured seq number reaches this value, the upstreaming will be started
    // which will start transmitting from the first sequence packet
    uint32_t                    capturedSeqNumToStartUpStreaming;
    bool                        isUpStreamingStarted;
    uint32_t                    usGapBetweenCapturedFrameAndTransmittedFrame;

    // code algorithm context
    gaf_codec_algorithm_context_t codec_alg_context[GAF_AUDIO_ASE_TOTAL_COUNT];

    int8_t                      tgt_vol;

} GAF_AUDIO_STREAM_CONTEXT_T;

typedef struct
{
    const char* codec;
    uint8_t con_lid;
    bool is_mobile;
    void* gaf_playback_status_mutex[GAF_AUDIO_ASE_TOTAL_COUNT];

    uint16_t bap_contextType;
    GAF_AUDIO_STREAM_CONTEXT_TYPE_E contextType;
    // see @GAF_AUDIO_STREAM_TYPE_E

    GAF_AUDIO_STREAM_COMMON_INFO_T  playbackInfo;
    GAF_AUDIO_STREAM_COMMON_INFO_T  captureInfo;
    GAF_AUDIO_TX_ALGO_CFG_T         tx_algo_cfg;
} GAF_AUDIO_STREAM_INFO_T;

typedef struct
{
    GAF_AUDIO_START_STREAM_FUNC     start_stream_func;
    GAF_AUDIO_INIT_STREAM_BUF_FUNC  init_stream_buf_func;
    GAF_AUDIO_STOP_STREAM_FUNC      stop_stream_func;

    GAF_AUDIO_STREAM_DMA_IRQ_HANDLER  playback_dma_irq_handler_func;
    GAF_AUDIO_STREAM_DMA_IRQ_HANDLER  capture_dma_irq_handler_func;
    GAF_AUDIO_DEINIT_STREAM_BUF_FUNC  deinit_stream_buf_func;

    GAF_AUDIO_START_STREAM_FUNC     playback_start_stream_func;
    GAF_AUDIO_INIT_STREAM_BUF_FUNC  playback_init_stream_buf_func;
    GAF_AUDIO_STOP_STREAM_FUNC      playback_stop_stream_func;
    GAF_AUDIO_DEINIT_STREAM_BUF_FUNC  playback_deinit_stream_buf_func;

    GAF_AUDIO_START_STREAM_FUNC     capture_start_stream_func;
    GAF_AUDIO_INIT_STREAM_BUF_FUNC  capture_init_stream_buf_func;
    GAF_AUDIO_STOP_STREAM_FUNC      capture_stop_stream_func;
    GAF_AUDIO_DEINIT_STREAM_BUF_FUNC  capture_deinit_stream_buf_func;

} GAF_AUDIO_STREAM_FUNC_LIST_T;

typedef struct
{
    GAF_AUDIO_DECODER_INIT_BUFFER   decoder_init_buf_func;
    GAF_AUDIO_DECODER_INIT_FUNC     decoder_init_func;
    GAF_AUDIO_DECODER_DEINIT_FUNC   decoder_deinit_func;
    GAF_AUDIO_DECODER_DECODE_FRAME_FUNC decoder_decode_frame_func;
    GAF_AUDIO_DECODER_DEINIT_BUFFER   decoder_deinit_buf_func;
} GAF_AUDIO_DECODER_FUNC_LIST_T;

typedef struct
{
    GAF_AUDIO_ENCODER_INIT_BUFFER   encoder_init_buf_func;
    GAF_AUDIO_ENCODER_INIT_FUNC     encoder_init_func;
    GAF_AUDIO_ENCODER_DEINIT_FUNC   encoder_deinit_func;
    GAF_AUDIO_ENCODER_ENCODE_FRAME_FUNC encoder_encode_frame_func;
    GAF_AUDIO_ENCODER_DEINIT_BUFFER   encoder_deinit_buf_func;
} GAF_AUDIO_ENCODER_FUNC_LIST_T;

typedef struct
{
    GAF_AUDIO_STREAM_FUNC_LIST_T    stream_func_list;
    const GAF_AUDIO_DECODER_FUNC_LIST_T*  decoder_func_list;
    const GAF_AUDIO_ENCODER_FUNC_LIST_T*  encoder_func_list;
} GAF_AUDIO_FUNC_LIST_T;

typedef struct
{
    // changing value along with streaming
    GAF_AUDIO_STREAM_CONTEXT_T  stream_context;

    // constant value configured when starting stream
    GAF_AUDIO_STREAM_INFO_T     stream_info;

    GAF_AUDIO_FUNC_LIST_T*      func_list;

} GAF_AUDIO_STREAM_ENV_T;

typedef struct
{
    // Number sink ase - 1
    uint8_t playback_ase_id[GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT];
    uint8_t capture_ase_id[GAF_AUDIO_USED_ASE_PER_DIR_MAX_CNT];

    // from GAF_AUDIO_STREAM_TYPE_E
    // when startedStreamTypes becomes the same as included_stream_types,
    // start the context
    uint8_t startedStreamTypes;
} GAF_MEDIA_DWELLING_INFO_T;

typedef struct
{
    // from GAF_AUDIO_STREAM_TYPE_E
    uint8_t included_stream_type;
    // from GAF_AUDIO_TRIGGER_STREAM_TYPE_E
    uint8_t trigger_stream_type;

    uint8_t playback_ase_count;
    uint8_t capture_ase_count;
} GAF_MEDIA_STREAM_TYPE_OPERATION_RULE_T;

typedef void (*GAF_ADUIO_STREAM_INFO_GET)(GAF_AUDIO_STREAM_ENV_T **pStreamEnv);

typedef struct
{
    /// After SINK received a ** codec packet
    gaf_stream_recv_data_custom_cb encoded_packet_recv_cb;
    /// After SINK decode a ** codec packet to raw pcm data
    gaf_stream_playback_dma_irq_custom_cb decoded_raw_data_cb;
    /// Before SRC prepared for sending a ** codec packet
    gaf_stream_send_data_custom_cb encoded_packet_send_cb;
    /// Before SRC prepared for encoding a pcm to ** codec packet
    gaf_stream_capture_dma_irq_custom_cb raw_pcm_data_cb;
} GAF_STREAM_COMMON_CUSTOM_DATA_HANDLER_FUNC_T ;

/****************************function declaration***************************/
#ifdef __cplusplus
extern "C"{
#endif

void gaf_stream_register_running_stream_ref(uint8_t con_lid, GAF_AUDIO_STREAM_ENV_T* pStreamEnv);
void gaf_stream_register_retrigger_callback(GAF_ADUIO_STREAM_RETRIGGER retrigger_cb);
void gaf_mobile_stream_register_retrigger_callback(GAF_ADUIO_STREAM_RETRIGGER retrigger_cb);
void gaf_stream_heap_init(void *begin_addr, uint32_t size);
void *gaf_stream_heap_malloc(uint32_t size);
void *gaf_stream_heap_cmalloc(uint32_t size);
#ifdef GAF_CODEC_CROSS_CORE
void gaf_m55_stream_encoder_data_free(void *packet);
void gaf_m55_stream_encoder_heap_free(void *rmem);
void *gaf_stream_pcm_data_frame_malloc(uint32_t packet_len);
#endif
void *gaf_stream_heap_realloc(void *rmem, uint32_t newsize);
void gaf_stream_heap_free(void *rmem);
void gaf_stream_data_free(void *packet);
void *gaf_stream_data_frame_malloc(uint32_t packet_len);
int gaf_buffer_mutex_lock(void *mutex);
int gaf_buffer_mutex_unlock(void *mutex);
int gaf_playback_status_mutex_lock(void *mutex);
int gaf_playback_status_mutex_unlock(void *mutex);
list_node_t *gaf_list_begin(gaf_stream_buff_list_t *list_info);
list_node_t *gaf_list_end(gaf_stream_buff_list_t *list_info);
uint32_t gaf_list_length(gaf_stream_buff_list_t *list_info);
void *gaf_list_node(gaf_stream_buff_list_t *list_info);
void *gaf_list_back(gaf_stream_buff_list_t *list_info);
list_node_t *gaf_list_next(gaf_stream_buff_list_t *list_info);
bool gaf_list_only_remove_node(gaf_stream_buff_list_t *list_info, void *data);
bool gaf_list_remove(gaf_stream_buff_list_t *list_info, void *data);
bool gaf_list_remove_generic(gaf_stream_buff_list_t *list_info, void *data);
bool gaf_list_append(gaf_stream_buff_list_t *list_info, void *data);
void gaf_list_clear(gaf_stream_buff_list_t *list_info);
void gaf_list_free(gaf_stream_buff_list_t *list_info);
void gaf_list_new(gaf_stream_buff_list_t *list_info, const osMutexDef_t *mutex_def, list_free_cb callback, list_mempool_zmalloc zmalloc, list_mempool_free free);
uint32_t gaf_stream_common_sample_freq_parse(uint8_t sample_freq);
float gaf_stream_common_frame_duration_parse(uint8_t frame_duration);
const char* gaf_stream_common_print_code_type(uint8_t codec_id);
const char* gaf_stream_common_print_context(uint16_t context_bf);
void gaf_stream_common_clr_trigger(uint8_t triChannel);
void gaf_stream_common_update_playback_stream_state(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, GAF_PLAYBACK_STREAM_STATE_E newState);
void gaf_stream_common_update_capture_stream_state(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, GAF_CAPTURE_STREAM_STATE_E newState);
const char* gaf_stream_common_get_capture_stream_state(GAF_CAPTURE_STREAM_STATE_E capture_stream_state);
void gaf_stream_common_set_playback_trigger_time(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint32_t tg_tick);
void gaf_stream_common_set_playback_trigger_time_generic(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint8_t dstStreamType, uint32_t tg_tick);
void gaf_stream_common_set_capture_trigger_time(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint32_t tg_tick);
void gaf_stream_common_set_capture_trigger_time_generic(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint8_t srcStreamType, uint32_t tg_tick);
void gaf_stream_common_capture_timestamp_checker(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint32_t dmaIrqHappeningTimeUs);
void gaf_stream_common_playback_timestamp_checker(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint32_t dmaIrqHappeningTimeUs);
gaf_media_data_t *gaf_stream_common_get_packet(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint8_t list_index, uint32_t dmaIrqHappeningTimeUs);
void gaf_stream_common_updated_expeceted_playback_seq_and_time(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, uint8_t list_idx, uint32_t dmaIrqHappeningTimeUs);
gaf_media_data_t* gaf_stream_common_store_received_packet(void* _pStreamEnv, uint8_t list_index, gaf_media_data_t *header);
bool gaf_stream_common_store_received_pcm_packet(void* _pStreamEnv, uint32_t dmairqhappentime, uint8_t* ptrBuf, uint32_t length);
void gaf_stream_common_register_func_list(GAF_AUDIO_STREAM_ENV_T* pStreamEnv, GAF_AUDIO_FUNC_LIST_T* funcList);
void gaf_stream_common_trigger_sync_capture(
    GAF_AUDIO_STREAM_CONTEXT_TYPE_E streamContext, uint32_t master_clk_cnt,
    int32_t usSinceLatestAnchorPoint, uint32_t triggertimeUs);
void gaf_stream_common_start_sync_capture(GAF_AUDIO_STREAM_ENV_T* pStreamEnv);
void gaf_stream_common_sync_us_since_latest_anchor_point(GAF_AUDIO_STREAM_ENV_T* pStreamEnv);
bool gaf_stream_get_prefill_status(void);
void gaf_stream_set_prefill_status(bool is_doing);
uint32_t gaf_media_common_get_latest_tx_iso_evt_timestamp(GAF_AUDIO_STREAM_ENV_T* pStreamEnv);
uint32_t gaf_media_common_get_latest_rx_iso_evt_timestamp(GAF_AUDIO_STREAM_ENV_T* pStreamEnv);
uint8_t gaf_media_common_get_ase_chan_lid_from_iso_channel(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
                                                        uint8_t direction, uint8_t iso_channel);
#if defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
gaf_media_data_t *gaf_stream_common_get_combined_packet_from_multi_channels(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
                                                                            uint32_t dmaIrqHappeningTimeUs);
void gaf_stream_common_update_multi_channel_expect_seq_and_time(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
                                                                   uint32_t dmaIrqHappeningTimeUs);
#endif

#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
typedef void (*gaf_stream_common_plc_packet_occurs_cb)(uint16_t pkt_seq_nb,
    GAF_ISO_PKT_STATUS_E pkt_status, uint32_t time_stamp, uint32_t dmaIrqHappenTime);

typedef void (*gaf_stream_common_get_packet_cb)(uint8_t con_lid, uint16_t pkt_seq_nb,
        GAF_ISO_PKT_STATUS_E pkt_status, uint32_t time_stamp, uint32_t dmaIrqHappenTime);
typedef void (*gaf_stream_common_dma_irq_happens_cb)(uint8_t con_lid, uint32_t dmaChunkIntervalUs);

typedef void(*gaf_stream_dma_irq_cb) (uint8_t con_lid, uint32_t dmaIrqHappeningTimeUs, uint16_t seq_num);

void gaf_stream_common_register_stream_plc_packet_occurs_cb(gaf_stream_common_plc_packet_occurs_cb func);
void gaf_stream_common_register_stream_get_packet_cb(gaf_stream_common_get_packet_cb func);
void gaf_stream_common_register_dma_irq_happens_cb(gaf_stream_common_dma_irq_happens_cb func);

#endif   //IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED

uint8_t gaf_stream_common_get_ase_idx_in_ase_lid_list(uint8_t* ase_lid_list, uint8_t ase_lid);
uint8_t gaf_stream_common_get_valid_idx_in_ase_lid_list(uint8_t* ase_lid_list);
/**********************************GAF CUSTOM**********************************/
/**
 * @brief Set custom data callback handler
 * 
 * @param gaf_user_case 
 * @param func_list 
 */
void gaf_stream_common_set_custom_data_handler(GAF_STREAM_USER_CASE_E gaf_user_case,
                                               const GAF_STREAM_COMMON_CUSTOM_DATA_HANDLER_FUNC_T *func_list);
/**
* @brief Get custom data callback handler
* 
* @param gaf_user_case 
* @return const GAF_STREAM_COMMON_CUSTOM_DATA_HANDLER_FUNC_T* 
*/
const GAF_STREAM_COMMON_CUSTOM_DATA_HANDLER_FUNC_T
     *gaf_stream_common_get_custom_data_handler(GAF_STREAM_USER_CASE_E gaf_user_case);

/**
 * @brief Set custom presDelay
 * 
 * @param presDelayUs presentation delay in us
 */
void gaf_stream_common_set_custom_presdelay_us(uint32_t presDelayUs);

/**
 * @brief Get custom set presDelay in us
 * 
 * @return uint32_t 
 */
uint32_t gaf_stream_common_get_custom_presdelay_us(void);

void gaf_stream_common_tws_sync_capture_trigger_handler(uint16_t rsp_seq, uint8_t* data, uint16_t len);

void gaf_stream_common_tws_sync_capture_trigger_rsp_timeout_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void gaf_stream_common_tws_sync_capture_trigger_rsp_handler(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);

void gaf_stream_common_tws_sync_us_since_latest_anchor_point_handler(uint16_t rsp_seq, uint8_t* data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GAF_MEDIA_COMMON_H__ */

