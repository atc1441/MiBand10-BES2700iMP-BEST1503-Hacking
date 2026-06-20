/**
 * @file aob_ux_stm.cpp
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

/*****************************header include********************************/
#include "bluetooth_bt_api.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "app_trace_rx.h"
#include "bluetooth_bt_api.h"
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
#include "gaf_codec_lc3.h"
#include "gaf_codec_cc_bth.h"

#include "ble_audio_earphone_info.h"

#include "smf_api.h"
#ifdef AOB_CODEC_CP
#include "cp_accel.h"
#include "norflash_api.h"
#endif /// AOB_CODEC_CP
#ifdef GAF_CODEC_CROSS_CORE
#include "gaf_codec_cc_common.h"
#include "gaf_media_common.h"
#include "app_dsp_m55.h"
#include "mcu_dsp_m55_app.h"
#include "rwble_config.h"
#endif /// GAF_CODEC_CROSS_CORE
#if defined(GAF_LC3_BES_PLC_ON)
#include "sbcplc.h"
#elif defined(GAF_LC3_MUSIC_PLC_ON)
#include "plc.h"
#endif

#ifdef BLE_SMF
static POSSIBLY_UNUSED lc3_buf_entry_t lc3_buf_entry[GAF_CODEC_LC3_BUF_ENTER_CONT];
#endif

#ifndef BLE_SMF
/*********************external function declaration*************************/

/*********************private  function declaration*************************/
#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
static void aob_stream_lc3_encoder_init(void *_p_lc3_enc_info);
#endif
#ifndef GAF_DECODER_CROSS_CORE_USE_M55
static void aob_stream_lc3_decoder_init(void *_p_lc3_dec_info);
#endif

#ifdef AOB_CODEC_CP
static void *gaf_codec_heap_malloc(uint32_t size);
#endif

typedef struct
{
    uint32_t start_time_in_ms;
    uint32_t total_time_in_us;
    bool codec_started;
    uint32_t codec_mips;
    uint32_t total_etime_in_us;
} aob_codec_time_info_t;

/************************private variable defination************************/
#if AOB_CALCULATE_CODEC_MIPS
aob_codec_time_info_t aob_decode_time_info = {0};
aob_codec_time_info_t aob_encode_time_info = {0};
#endif

#if defined(GAF_LC3_BES_PLC_ON)
static PLC_State *lc3_plc_state1 = NULL;
static POSSIBLY_UNUSED PLC_State *lc3_plc_state2 = NULL;
static float *lc3_cos_buf = NULL;
static int lc3_packeet_len = 0;
static int lc3_smooth_len = 0;
static int plc_is_16bit = 0;
#elif defined(GAF_LC3_MUSIC_PLC_ON)
static void *lc3_plc_state = NULL;
static PlcParam lc3_plc_param;
#define MUSIC_PLC_DELAY 4
#endif

/************************private macro defination***************************/
#define AOB_CALCULATE_CODEC_MIPS       0
#define LC3_AUDIO_UPLOAD_DUMP          0
#define LC3_AUDIO_DOWNLOAD_DUMP        0

/************************private type defination****************************/

static POSSIBLY_UNUSED lc3_buf_entry_t lc3_buf_entry[GAF_CODEC_LC3_BUF_ENTER_CONT];

#ifdef AOB_CODEC_CP
static void* lc3_alloc(void*pool,unsigned size){
    return gaf_codec_heap_malloc(size);
}
static void lc3_free(void*pool,void* data){
    return ;
}
#else
static void lc3_alloc_data_store(uint8_t* data, unsigned size)
{
    for (uint32_t i = 0; i < GAF_CODEC_LC3_BUF_ENTER_CONT; i++) {
        if (!lc3_buf_entry[i].is_used) {
            lc3_buf_entry[i].data = data;
            lc3_buf_entry[i].size = size;
            lc3_buf_entry[i].is_used = true;
            LOG_I("lc3 store alloc 0x%p)", data);
            break;
        }
    }
}

static uint8_t* lc3_get_stored_data(unsigned size)
{
    uint8_t *data=0;
    for (uint32_t i = 0; i < GAF_CODEC_LC3_BUF_ENTER_CONT; i++) {
        if ((lc3_buf_entry[i].size == size) && !lc3_buf_entry[i].is_used) {
            data = lc3_buf_entry[i].data;
            lc3_buf_entry[i].is_used = true;
            LOG_I("lc3 not realloc 0x%p)", data);
            return data;
        }
    }
    return NULL;
}

void lc3_alloc_data_free(void)
{
    for (uint32_t i = 0; i < GAF_CODEC_LC3_BUF_ENTER_CONT; i++) {
        lc3_buf_entry[i].is_used = false;
        lc3_buf_entry[i].size = 0;
        lc3_buf_entry[i].data = NULL;
    }
}

static void* lc3_alloc(void*pool,unsigned size){
    LOG_I("lc3_alloc(%d)",size);

    uint8_t *data = lc3_get_stored_data(size);
    if (data != NULL) {
        return data;
    }

    app_audio_mempool_get_buff(&data, size);
    lc3_alloc_data_store(data, size);

    return data;
}
static void lc3_free(void*pool,void* data){
    LOG_I("lc3_free");
    for (uint32_t i = 0; i < 4; i++) {
        if (lc3_buf_entry[i].is_used && (lc3_buf_entry[i].data == data)) {
            lc3_buf_entry[i].is_used = false;
            break;
        }
    }

    return ;
}
#endif

#if defined(GAF_LC3_MUSIC_PLC_ON)
int32_t music_plc_delay_get()
{
    return MUSIC_PLC_DELAY;
}
#endif

POSSIBLY_UNUSED static void lc3_overlay(LC3_Enc_Info* info){
#if 0 // LC3 work together with call algo and need to overlay Call algo firstly
    APP_OVERLAY_ID_T id = info->is_lc3plus ? APP_OVERLAY_LC3PLUS : APP_OVERLAY_LC3;
    LOG_I("load overlay(%d)",id);
    app_overlay_select(id);
#endif
}

#ifdef AOB_CODEC_CP
typedef enum  {
    GAF_CODEC_CP_STATE_IDLE = 0,
    GAF_CODEC_CP_STATE_IN_DECODING = 1,
    GAF_CODEC_CP_STATE_IN_ENCODING = 2,
} GAF_CODEC_CP_STATE_E;

typedef enum
{
    GAF_CODEC_CP_PROCESSING_TYPE_DECODE = 0,
    GAF_CODEC_CP_PROCESSING_TYPE_ENCODE = 1,
} GAF_CODEC_CP_PROCESSING_TYPE_E;

typedef struct
{
    GAF_CODEC_CP_PROCESSING_TYPE_E processing_type;
    uint32_t processor_func;
    void*   lc3_dec_instance;
    void*   scratch;
    void*   input_bytes;
    int32_t num_bytes;
    void*   output_samples;
#if defined(GAF_LC3_MUSIC_PLC_ON)
    void*   plc_func;
    void*   lc3_plc_state;
    int32_t osize;
#endif
    int32_t bfi_ext;
} GAF_CODEC_CP_DECODING_INFO_T;

typedef struct
{
    GAF_CODEC_CP_PROCESSING_TYPE_E processing_type;
    uint32_t processor_func;
    void*   lc3_enc_instance;
    void*   scratch;
    void*   input_samples;
    void*   output_bytes;
    int32_t num_bytes;
} GAF_CODEC_CP_ENCODING_INFO_T;

typedef union
{
    GAF_CODEC_CP_PROCESSING_TYPE_E processing_type;
    GAF_CODEC_CP_DECODING_INFO_T cp_decoding_info;
    GAF_CODEC_CP_ENCODING_INFO_T cp_encoding_info;
} GAF_CODEC_CP_PROCESSING_INFO_T;

#define GAF_CODEC_WAIT_CP_TIMEOUT_US    3000000 // 3 seconds
#define GAF_CODEC_WAIT_CP_US_PER_ROUND  10
#define GAF_CODEC_WAIT_CP_TIMEOUT_ROUNDS ((GAF_CODEC_WAIT_CP_TIMEOUT_US)/(GAF_CODEC_WAIT_CP_US_PER_ROUND))

static CP_DATA_LOC GAF_CODEC_CP_PROCESSING_INFO_T gaf_codec_cp_processing_info;
static CP_DATA_LOC int gaf_codec_cp_processing_result;

static CP_DATA_LOC GAF_CODEC_CP_STATE_E gaf_codec_cp_state = GAF_CODEC_CP_STATE_IDLE;


/***** M55 section ******/
#ifdef GAF_CODEC_CROSS_CORE
typedef struct
{
    uint32_t processor_func;
    void*   lc3_dec_instance;
    void*   scratch;
    void*   input_bytes;
    int32_t num_bytes;
    void*   output_samples;
    int32_t bfi_ext;
} GAF_CODEC_M55_DECODING_INFO_T;

typedef struct
{
    uint32_t processor_func;
    void*    lc3_enc_instance;
    void*    scratch;
    void*    input_samples;
    void*    output_bytes;
    int32_t* num_bytes;
} GAF_CODEC_M55_ENCODING_INFO_T;
#endif

static heap_handle_t gaf_codec_cp_heap = NULL;
static uint8_t gaf_codec_cp_heap_init_times = 0;

static uint8_t gaf_codec_cp_user_cnt = 0;

typedef int32_t(*cb_encode)(void* instance, void* scratch, void* input_samples, void* output_bytes, int32_t num_bytes);
typedef int32_t(*cb_decode)(void* instance, void* scratch, void* input_bytes, int32_t num_bytes, void* output_samples, int32_t bfi_ext);

/**********************private function declaration*************************/
#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
static void aob_stream_lc3_encoder_init(void *_p_lc3_enc_info);
#endif
#ifndef GAF_DECODER_CROSS_CORE_USE_M55
static void aob_stream_lc3_decoder_init(void *_p_lc3_dec_info);
#endif

static unsigned int _cp_aob_codec_main(uint8_t event);

static struct cp_task_desc TASK_DESC_AOB_CODEC = {
    CP_ACCEL_STATE_CLOSED,_cp_aob_codec_main, NULL, NULL, NULL};

static void gaf_codec_wait_for_cp_idle(void)
{
    uint32_t wait_cnt = 0;
    while (GAF_CODEC_CP_STATE_IDLE != gaf_codec_cp_state)
    {
        hal_sys_timer_delay_us(GAF_CODEC_WAIT_CP_US_PER_ROUND);

        if (wait_cnt++ > GAF_CODEC_WAIT_CP_TIMEOUT_ROUNDS)
        {
            ASSERT(0, "gaf codec cp is hung %d", gaf_codec_cp_state);
        }
    }
}

static int32_t gaf_codec_audio_decode_over_cp(
        GAF_CODEC_CP_PROCESSING_TYPE_E processor_type,
        uint32_t processor_func,
        void*   lc3_dec_instance,
        void*   scratch,
        void*   input_bytes,
        int32_t num_bytes,
        void*   output_samples,
#if defined(GAF_LC3_MUSIC_PLC_ON)
        void*   plc_func,
        void*   lc3_plc_state,
        int32_t osize,
#endif
        int32_t bfi_ext)
{
    gaf_codec_wait_for_cp_idle();
    gaf_codec_cp_processing_info.processing_type = processor_type;
    GAF_CODEC_CP_DECODING_INFO_T* pDecodingInfo =
        &(gaf_codec_cp_processing_info.cp_decoding_info);
    pDecodingInfo->processor_func = processor_func;
    pDecodingInfo->lc3_dec_instance = lc3_dec_instance;
    pDecodingInfo->scratch = scratch;
    pDecodingInfo->input_bytes = input_bytes;
    pDecodingInfo->num_bytes = num_bytes;
    pDecodingInfo->output_samples = output_samples;
#if defined(GAF_LC3_MUSIC_PLC_ON)
    pDecodingInfo->plc_func = plc_func;
    pDecodingInfo->lc3_plc_state = lc3_plc_state;
    pDecodingInfo->osize = osize;
#endif
    pDecodingInfo->bfi_ext = bfi_ext;
    gaf_codec_cp_state = GAF_CODEC_CP_STATE_IN_DECODING;
    cp_accel_send_event_mcu2cp(CP_BUILD_ID(CP_TASK_AOB_CODEC, CP_EVENT_AOB_CODEC_PROCESSING));

    gaf_codec_wait_for_cp_idle();

    return gaf_codec_cp_processing_result;
}

static int32_t gaf_codec_audio_encode_over_cp(
        GAF_CODEC_CP_PROCESSING_TYPE_E processor_type,
        uint32_t processor_func,
        void*   lc3_enc_instance,
        void*   scratch,
        void*   input_samples,
        void*   output_bytes,
        int32_t num_bytes)
{
    gaf_codec_wait_for_cp_idle();
    gaf_codec_cp_processing_info.processing_type = processor_type;
    GAF_CODEC_CP_ENCODING_INFO_T* pEncodingInfo =
        &(gaf_codec_cp_processing_info.cp_encoding_info);
    pEncodingInfo->processor_func = processor_func;
    pEncodingInfo->lc3_enc_instance = lc3_enc_instance;
    pEncodingInfo->scratch = scratch;
    pEncodingInfo->input_samples = input_samples;
    pEncodingInfo->output_bytes = output_bytes;
    pEncodingInfo->num_bytes = num_bytes;

    gaf_codec_cp_state = GAF_CODEC_CP_STATE_IN_ENCODING;
    cp_accel_send_event_mcu2cp(CP_BUILD_ID(CP_TASK_AOB_CODEC, CP_EVENT_AOB_CODEC_PROCESSING));

    gaf_codec_wait_for_cp_idle();

    return gaf_codec_cp_processing_result;
}

static void *gaf_codec_heap_malloc(uint32_t size)
{
    void *ptr = NULL;

    if (gaf_codec_cp_heap) {
        ptr = heap_malloc(gaf_codec_cp_heap, size);
        LOG_I("gaf cp heap allocate %d", size);
    }
    if (ptr == NULL) {
        app_audio_mempool_get_buff((uint8_t **)&ptr, size);
        ASSERT(ptr, "%s: Failed to alloc mem in gaf_codec_cp_heap: %u", __func__, size);
    }
    return ptr;
}

#if defined(AOB_CODEC_CP) && !defined(UNIFY_HEAP_ENABLED)
static void gaf_codec_cp_heap_init(void)
{
    gaf_codec_cp_heap_init_times++;

    if (NULL == gaf_codec_cp_heap)
    {
        uint8_t *heap_addr;
        uint32_t heap_size;

        cp_pool_init();
        heap_size = cp_pool_total_size();
        cp_pool_get_buff(&heap_addr, heap_size);
        ASSERT(heap_addr, "%s: Failed to get buffer from CP pool: size=%u", __func__, heap_size);

        if (heap_addr) {
            gaf_codec_cp_heap = heap_register(heap_addr, heap_size);
            ASSERT(gaf_codec_cp_heap, "%s: Failed to register CP heap: ptr=%p size=%u",
                __func__, heap_addr, heap_size);
        }
    }
}
#endif

static void gaf_codec_cp_heap_deinit(void)
{
    if (gaf_codec_cp_heap_init_times > 0)
    {
        gaf_codec_cp_heap_init_times--;
        if (0 == gaf_codec_cp_heap_init_times)
        {
            gaf_codec_cp_heap = NULL;
        }
    }
}

CP_TEXT_SRAM_LOC
static unsigned int _cp_aob_codec_main(uint8_t event)
{
    switch (event)
    {
        case CP_EVENT_AOB_CODEC_PROCESSING:
        {
            gaf_codec_cp_processing_result = -1;
            switch (gaf_codec_cp_processing_info.processing_type)
            {
                case GAF_CODEC_CP_PROCESSING_TYPE_DECODE:
                {
                    GAF_CODEC_CP_DECODING_INFO_T* pDecodingInfo =
                        &(gaf_codec_cp_processing_info.cp_decoding_info);
                    ASSERT(pDecodingInfo, "cp_decoding_info is empty");
                    gaf_codec_cp_processing_result =
                        ((cb_decode)pDecodingInfo->processor_func)(
                            pDecodingInfo->lc3_dec_instance,
                            pDecodingInfo->scratch,
                            pDecodingInfo->input_bytes,
                            pDecodingInfo->num_bytes,
                            pDecodingInfo->output_samples,
                            pDecodingInfo->bfi_ext);
#if defined(GAF_LC3_MUSIC_PLC_ON)
                    if(gaf_codec_cp_processing_result == 0)
                    {
                        if(pDecodingInfo->bfi_ext)
                        {
                            gaf_codec_cp_processing_result = 
                                ((cb_plc)pDecodingInfo->plc_func)(pDecodingInfo->lc3_plc_state,
                                                NULL,
                                                0,
                                                NULL,
                                                (int8_t*)pDecodingInfo->output_samples,
                                                NULL,
                                                true);
                        }else{
                            gaf_codec_cp_processing_result = 
                                ((cb_plc)pDecodingInfo->plc_func)(pDecodingInfo->lc3_plc_state,
                                                (int8_t*)pDecodingInfo->output_samples,
                                                pDecodingInfo->osize,
                                                NULL,
                                                (int8_t*)pDecodingInfo->output_samples,
                                                NULL,
                                                false);
                        }
                    }
#endif
                    break;
                }
                case GAF_CODEC_CP_PROCESSING_TYPE_ENCODE:
                {
                    GAF_CODEC_CP_ENCODING_INFO_T* pEncodingInfo =
                        &(gaf_codec_cp_processing_info.cp_encoding_info);
                    gaf_codec_cp_processing_result =
                        ((cb_encode)pEncodingInfo->processor_func)(
                            pEncodingInfo->lc3_enc_instance,
                            pEncodingInfo->scratch,
                            pEncodingInfo->input_samples,
                            pEncodingInfo->output_bytes,
                            pEncodingInfo->num_bytes);
                    break;
                }
                default:
                    break;
            }
            gaf_codec_cp_state = GAF_CODEC_CP_STATE_IDLE;
            break;
        }
        default:
            break;
    }
    return 0;
}

static void gaf_audio_lc3_stop_cp_process(void)
{
    if (gaf_codec_cp_user_cnt > 0)
    {
        gaf_codec_cp_user_cnt--;
        if (0 == gaf_codec_cp_user_cnt)
        {
            LOG_I("lc3 stop codec processing in cp:%s", __func__);
            cp_accel_close(CP_TASK_AOB_CODEC);
        }
    }
}

static void gaf_audio_lc3_start_cp_process(void)
{
    if (0 == gaf_codec_cp_user_cnt)
    {
        norflash_api_flush_disable(NORFLASH_API_USER_CP,(uint32_t)cp_accel_init_done, false);
        cp_accel_open(CP_TASK_AOB_CODEC, &TASK_DESC_AOB_CODEC);
        while (cp_accel_init_done() == false)
        {
            LOG_D("[%s] Delay...", __func__);
            osDelay(1);
        }
        norflash_api_flush_enable(NORFLASH_API_USER_CP);

        LOG_I("[%s]", __func__);
    }
    gaf_codec_cp_user_cnt++;
}
#endif

#ifndef GAF_CODEC_CROSS_CORE
/**
 ****************************************************************************************
 * @brief It is used to adjust the media bth freqency, when close m55 core.
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
enum APP_SYSFREQ_FREQ_T gaf_audio_media_decoder_adjust_bth_freq_no_m55(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_)
{
    enum APP_SYSFREQ_FREQ_T gaf_audio_bth_base_freq_non_m55 = _base_freq_;
    uint32_t lc3_bitrate = frame_size * 80000 / frame_ms;

    if (44100 == sample_rate){
        lc3_bitrate = lc3_bitrate * 44100 / 48000;
    }
    LOG_I("BTH_LC3_bitrate:%d", lc3_bitrate);

    if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_15M;
    #else
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_26M;
    #endif
    }
    else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_26M;
    }
    else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_26M;
    }
    else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M	
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_30M;
    #else
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_52M;
    #endif		
    }
    else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M	
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_30M;
    #else
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_52M;
    #endif	
    }
    else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M	
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_30M;
    #else
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_52M;
    #endif	
    }
    else if ((88200 == lc3_bitrate) && (44100 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M	
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_48M;
    #else
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_52M;
    #endif	
    }
    else if ((96000 == lc3_bitrate) && (48000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M	
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_48M;
    #else
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_52M;
    #endif	
    }
    else if ((96000 < lc3_bitrate) && (48000 < sample_rate))
    {
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_78M;
    }

    if (channel >= 2)
    {
#ifdef HAL_CMU_FREQ_15M
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_144M;
#else
        gaf_audio_bth_base_freq_non_m55 = APP_SYSFREQ_208M;
#endif
    }

    return gaf_audio_bth_base_freq_non_m55;
}

/**
 ****************************************************************************************
 * @brief It is used to adjust the flexible bth freqency, when close m55 core.
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
enum APP_SYSFREQ_FREQ_T gaf_audio_flexible_decoder_adjust_bth_freq_no_m55(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_)
{
    enum APP_SYSFREQ_FREQ_T gaf_audio_base_freq = _base_freq_;
    uint32_t lc3_bitrate = frame_size * 80000 / frame_ms;

    if (44100 == sample_rate){
        lc3_bitrate = lc3_bitrate * 44100 / 48000;
    }
    LOG_I("LC3_bitrate:%d", lc3_bitrate);

    if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_12M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    #endif	
    }
    else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_15M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    #endif	
    }
    else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_15M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    #endif	
    }
    else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_15M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    #endif	
    }
    else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    }
    else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    }
    else if ((88200 == lc3_bitrate) && (44100 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_30M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_52M;
    #endif	
    }
    else if ((96000 == lc3_bitrate) && (48000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_30M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_52M;
    #endif	
    }
    else if ((96000 < lc3_bitrate) && (48000 < sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_48M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_52M;
    #endif	
    }
    if (channel >= 2)
    {
#ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_144M;
#else
        gaf_audio_base_freq = APP_SYSFREQ_208M;
#endif
    }
    return gaf_audio_base_freq;
}

/**
 ****************************************************************************************
 * @brief It is used to adjust the flexible bth freqency, when close m55 core.
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
enum APP_SYSFREQ_FREQ_T gaf_audio_stream_encoder_adjust_bth_freq_no_m55(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_)
{
    enum APP_SYSFREQ_FREQ_T gaf_audio_m55_base_freq = _base_freq_;
    if (frame_ms == 0)
    {
        ASSERT(0, "%s, wrong Divided ", __func__);
    }
    uint32_t lc3_bitrate = frame_size * 80000 / frame_ms;

    if (44100 == sample_rate){
        lc3_bitrate = lc3_bitrate * 44100 / 48000;
    }
    LOG_I("bth_encoder_lc3_bitrate:%d sample_rate: %d", lc3_bitrate, sample_rate);

    if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_52M;
    }
    else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_104M;
    }
    else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_52M;
    }
    else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
    }
    else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
    }
    else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
    }
    else if ((64000 == lc3_bitrate) && (48000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_104M;
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
    if (channel >= 2)
    {
#ifdef HAL_CMU_FREQ_15M
        gaf_audio_m55_base_freq = APP_SYSFREQ_144M;
#else
        gaf_audio_m55_base_freq = APP_SYSFREQ_208M;
#endif
    }
    return gaf_audio_m55_base_freq;
}
#endif /// NON_GAF_CODEC_CROSS_CORE

POSSIBLY_UNUSED static uint32_t aob_lc3_get_sysfreq(void)
{
    enum HAL_CMU_FREQ_T cpu_freq = hal_sysfreq_get();

    switch(cpu_freq)
    {
        case HAL_CMU_FREQ_32K:
            return 0;
            break;
        case HAL_CMU_FREQ_26M:
            return 26;
            break;
        case HAL_CMU_FREQ_52M:
            return 52;
            break;
        case HAL_CMU_FREQ_78M:
            return 78;
            break;
        case HAL_CMU_FREQ_104M:
            return 104;
            break;
        case HAL_CMU_FREQ_208M:
            return 208;
            break;
        default:
            return 0;
            break;
    }
}

#ifndef GAF_DECODER_CROSS_CORE_USE_M55
static void aob_stream_lc3_decoder_init(void *_p_lc3_dec_info)
{
    //app_overlay_select(APP_OVERLAY_LC3);//load overlay by callback
    LC3_Dec_Info *p_lc3_dec_info = (LC3_Dec_Info *)_p_lc3_dec_info;
    uint32_t decoder_size = 0;
    uint32_t scratch_size = 0;
    int err = LC3_API_OK;

    LOG_I("%s", __FUNCTION__);
#if AOB_CALCULATE_CODEC_MIPS
    memset(&aob_decode_time_info, 0, sizeof(aob_codec_time_info_t));
#endif
    //decoder_size = lc3_api_dec_get_size(p_lc3_dec_info->sample_rate,
    //    p_lc3_dec_info->channels, p_lc3_dec_info->plcMeth);
    //memset((void*)p_lc3_dec_info->decode, 0, decoder_size);
    err = lc3_api_decoder_init(p_lc3_dec_info);

    //scratch_size = lc3_api_dec_get_scratch_size(p_lc3_dec_info->decode);
    //memset((void*)p_lc3_dec_info->scratch, 0, scratch_size);

    ASSERT(LC3_API_OK == err, "%s err %d", __func__, err);

    /* Print info */
    LOG_I("[lc3]");
    LOG_I("Decoder size:     %d", decoder_size);
    LOG_I("Scratch size:     %d", scratch_size);
    LOG_I("Bits:             %d", p_lc3_dec_info->bitwidth);
    LOG_I("Bits align:       %d", p_lc3_dec_info->bitalign);
    LOG_I("Sample rate:      %d", p_lc3_dec_info->sample_rate);
    LOG_I("Channels:         %d", p_lc3_dec_info->channels);
    LOG_I("Frame samples:    %d", p_lc3_dec_info->frame_samples);
    LOG_I("Frame length:     %d", p_lc3_dec_info->frame_size);
    LOG_I("PLC mode:         %d", p_lc3_dec_info->plcMeth);
    LOG_I("Bitrate:          %d", p_lc3_dec_info->bitrate);
}
#endif

static int aob_stream_lc3_decode_bits(void *lc3_dec_info, void *input_bytes, int32_t num_bytes,
    void *output_samples, bool isPLC)
{
    int32_t bfi_ext = 0;
    int32_t err = LC3_API_OK;
    LC3_Dec_Info *p_lc3_dec_info = (LC3_Dec_Info *)lc3_dec_info;
    ASSERT(p_lc3_dec_info, "p_lc3_dec_info is empty in %s ", __func__);
    if (0 == num_bytes||isPLC)
    {
        TRACE(0, "PLC num_bytes %d", num_bytes);
        bfi_ext = 1;
    }else{
        isPLC = lc3_api_decoder_bfi_detect(p_lc3_dec_info, (uint8_t *)input_bytes);
        if(isPLC)
        {
            bfi_ext = 1;
        }
    }
#if AOB_CALCULATE_CODEC_MIPS
    uint32_t cpu_freq = 0;
    POSSIBLY_UNUSED uint32_t stime = 0, etime = 0;
    uint32_t use_time_in_us = 0;
    if (!aob_decode_time_info.codec_started)
    {
        aob_decode_time_info.codec_started = true;
        aob_decode_time_info.total_time_in_us = 0;
        aob_decode_time_info.start_time_in_ms = hal_sys_timer_get();
    }

    stime = hal_sys_timer_get();
#endif

#ifdef AOB_CODEC_CP
    GAF_CODEC_CP_PROCESSING_TYPE_E sampleBits = GAF_CODEC_CP_PROCESSING_TYPE_DECODE;
    if(p_lc3_dec_info->bitwidth ==24 )
        sampleBits = GAF_CODEC_CP_PROCESSING_TYPE_DECODE;
    //else if(p_lc3_dec_info->bitwidth==32)
    //    bits = GAF_CODEC_CP_PROCESSING_TYPE_DECODE_32_BIT;
    if(p_lc3_dec_info->is_interlaced){
#if defined(GAF_LC3_MUSIC_PLC_ON)
        int32_t osize = p_lc3_dec_info->frame_samples * (p_lc3_dec_info->bitalign/8) * p_lc3_dec_info->channels;
        err = gaf_codec_audio_decode_over_cp(sampleBits,
            (uint32_t)p_lc3_dec_info->cb_decode_interlaced, p_lc3_dec_info, p_lc3_dec_info->scratch, input_bytes,
            num_bytes, output_samples, (void *)PlcRun, lc3_plc_state, osize, bfi_ext);
#else
        err = gaf_codec_audio_decode_over_cp(sampleBits,
            (uint32_t)p_lc3_dec_info->cb_decode_interlaced, p_lc3_dec_info, p_lc3_dec_info->scratch, input_bytes,
            num_bytes, output_samples, bfi_ext);
#endif
    }
    else{
        uint8_t* output = (uint8_t*)output_samples;
        uint32_t xlen = p_lc3_dec_info->frame_samples*p_lc3_dec_info->bitalign/8;
        void* pcms[4];
        pcms[0] = output;
        pcms[1] = output + xlen;
        pcms[2] = output + xlen*2;
        pcms[3] = output + xlen*3;
#if defined(GAF_LC3_MUSIC_PLC_ON)
        err = gaf_codec_audio_decode_over_cp(sampleBits,
            (uint32_t)p_lc3_dec_info->cb_decode, p_lc3_dec_info, p_lc3_dec_info->scratch, input_bytes,
            num_bytes, pcms, (void *)PlcRun, lc3_plc_state, xlen, bfi_ext);
#else
        err = gaf_codec_audio_decode_over_cp(sampleBits,
            (uint32_t)p_lc3_dec_info->cb_decode, p_lc3_dec_info, p_lc3_dec_info->scratch, input_bytes,
            num_bytes, pcms, bfi_ext);
#endif
    }
#else
    if(p_lc3_dec_info->is_interlaced){
        err = p_lc3_dec_info->cb_decode_interlaced(p_lc3_dec_info, p_lc3_dec_info->scratch, input_bytes,
            num_bytes, output_samples, bfi_ext);
    }
    else{
        uint8_t* output = (uint8_t*)output_samples;
        uint32_t xlen = p_lc3_dec_info->frame_samples*p_lc3_dec_info->bitalign/8;
        void* pcms[4];
        pcms[0] = output;
        pcms[1] = output + xlen;
        pcms[2] = output + xlen*2;
        pcms[3] = output + xlen*3;
        err = p_lc3_dec_info->cb_decode(p_lc3_dec_info, p_lc3_dec_info->scratch, input_bytes,
            num_bytes, pcms , bfi_ext);
    }
#if defined(GAF_LC3_BES_PLC_ON)
   if (p_lc3_dec_info->channels == 1)
   {
        if(plc_is_16bit == 1)
        {
            if (bfi_ext)
            {
                a2dp_plc_bad_frame(lc3_plc_state1, (short *)output_samples, (short *)output_samples, lc3_cos_buf, lc3_packeet_len, 1, 0);
            }
            else {
                a2dp_plc_good_frame(lc3_plc_state1, (short *)output_samples, (short *)output_samples, lc3_cos_buf, lc3_packeet_len, 1, 0);
            }
        }else{
            if (bfi_ext)
            {
                a2dp_plc_bad_frame_24bit(lc3_plc_state1, (int32_t *)output_samples, (int32_t *)output_samples, lc3_cos_buf, lc3_packeet_len, 1, 0);
            }
            else {
                a2dp_plc_good_frame_24bit(lc3_plc_state1, (int32_t *)output_samples, (int32_t *)output_samples, lc3_cos_buf, lc3_packeet_len, 1, 0);
            }
        }
    }
   else {
        if(plc_is_16bit == 1)
        {
            if (bfi_ext)
            {
                a2dp_plc_bad_frame(lc3_plc_state1, (short *)output_samples, (short *)output_samples, lc3_cos_buf, lc3_packeet_len, 2, 0);
                a2dp_plc_bad_frame(lc3_plc_state2, (short *)output_samples, (short *)output_samples, lc3_cos_buf, lc3_packeet_len, 2, 1);
            }
            else {
                a2dp_plc_good_frame(lc3_plc_state1, (short *)output_samples, (short *)output_samples, lc3_cos_buf, lc3_packeet_len, 2, 0);
                a2dp_plc_good_frame(lc3_plc_state2, (short *)output_samples, (short *)output_samples, lc3_cos_buf, lc3_packeet_len, 2, 1);
            }
        }else{
            if (bfi_ext)
            {
                a2dp_plc_bad_frame_24bit(lc3_plc_state1, (int32_t *)output_samples, (int32_t *)output_samples, lc3_cos_buf, lc3_packeet_len, 2, 0);
                a2dp_plc_bad_frame_24bit(lc3_plc_state2, (int32_t *)output_samples, (int32_t *)output_samples, lc3_cos_buf, lc3_packeet_len, 2, 1);
            }
            else {
                a2dp_plc_good_frame_24bit(lc3_plc_state1, (int32_t *)output_samples, (int32_t *)output_samples, lc3_cos_buf, lc3_packeet_len, 2, 0);
                a2dp_plc_good_frame_24bit(lc3_plc_state2, (int32_t *)output_samples, (int32_t *)output_samples, lc3_cos_buf, lc3_packeet_len, 2, 1);
            }
        }
    }
#elif defined(GAF_LC3_MUSIC_PLC_ON)
    PlcRet plcRet = PLC_RET_SUCCESS;
    if (bfi_ext)
    {
        plcRet = PlcRun(lc3_plc_state, NULL, 0, NULL, (int8_t*)output_samples, NULL, true);
    }
    else {
        plcRet = PlcRun(lc3_plc_state, (int8_t*)output_samples, \
        p_lc3_dec_info->frame_samples*sizeof(int32_t)*2, NULL, \
        (int8_t*)output_samples, NULL, false);
    }
    if(plcRet != PLC_RET_SUCCESS)
    {
        TRACE(1, "[%s] plc run fail, ret:%d", __FUNCTION__,plcRet);
    }
#endif
#endif

#if AOB_CALCULATE_CODEC_MIPS
    etime = hal_sys_timer_get();
    use_time_in_us = TICKS_TO_US(etime - stime);
    aob_decode_time_info.total_etime_in_us += p_lc3_dec_info->frame_dms * 100;
    aob_decode_time_info.total_time_in_us += use_time_in_us;
    cpu_freq = aob_lc3_get_sysfreq();
    if (aob_decode_time_info.total_etime_in_us)
    {
        aob_decode_time_info.codec_mips =
            cpu_freq * aob_decode_time_info.total_time_in_us / aob_decode_time_info.total_etime_in_us;
    }
    LOG_I("err %d ticks:%d time:%d us",
        err, (etime - stime), use_time_in_us);
    LOG_I("freq %d use:%d ms total:%d ms mips: %d M", cpu_freq,
        aob_decode_time_info.total_time_in_us/1000,
        aob_decode_time_info.total_etime_in_us/1000, aob_decode_time_info.codec_mips);
#endif
    if (err)
    {
        LOG_E("lc3 dec err:%d,num_bytes = %d\n",err,num_bytes);
    }
    LOG_D("[dec]%d->%d\n",p_lc3_dec_info->frame_size,p_lc3_dec_info->frame_samples);
    return (int)err;
}

#ifndef GAF_ENCODER_CROSS_CORE_USE_M55
static void aob_stream_lc3_encoder_init(void *_p_lc3_enc_info)
{
    LOG_I("%s", __func__);
    //app_overlay_select(APP_OVERLAY_LC3);//load overlay by callback
    LC3_Enc_Info *p_lc3_enc_info = (LC3_Enc_Info *)_p_lc3_enc_info;

    int err = LC3_API_OK;
#if AOB_CALCULATE_CODEC_MIPS
    /* Setup Encoder */
    memset(&aob_encode_time_info, 0, sizeof(aob_codec_time_info_t));
#endif
    err = lc3_api_encoder_init(p_lc3_enc_info);
    ASSERT(LC3_API_OK == err, "%s/%d err %d", __func__,__LINE__, err);
    p_lc3_enc_info->enabled_frame_buff = lc3_alloc(0,p_lc3_enc_info->frame_size);
    ASSERT(p_lc3_enc_info->instance, "%s/%d", __func__, __LINE__);
    ASSERT(p_lc3_enc_info->enabled_frame_buff, "%s/%d", __func__, __LINE__);
    //ASSERT(p_lc3_enc_info->cb_encode, "%s/%d", __func__, __LINE__);
    ASSERT(p_lc3_enc_info->cb_encode_interlaced, "%s/%d", __func__, __LINE__);
    ASSERT(p_lc3_enc_info->frame_samples, "%s/%d", __func__, __LINE__);
    ASSERT(p_lc3_enc_info->frame_size, "%s/%d", __func__, __LINE__);

    /* Print info */
    LOG_I("[lc3]");
    LOG_I("Encoder size:     %d", p_lc3_enc_info->instance_size);
    LOG_I("Scratch size:     %d", p_lc3_enc_info->scratch_size);
    LOG_I("Sample rate:      %d", p_lc3_enc_info->sample_rate);
    LOG_I("Channels:         %d", p_lc3_enc_info->channels);
    LOG_I("Frame samples:    %d", p_lc3_enc_info->frame_samples);
    LOG_I("Frame length:     %d", p_lc3_enc_info->frame_size);
    LOG_I("Output format:    %d bits", p_lc3_enc_info->bitwidth);
    LOG_I("Output format:    %d bits align", p_lc3_enc_info->bitalign);
    LOG_I("Target bitrate:   %d", p_lc3_enc_info->bitrate);
    //LOG_I("Real bitrate:     %d", p_lc3_enc_info->real_bitrate);
    //LOG_I("PLC mode:         %d", p_lc3_enc_info->plcMeth);
    LOG_I("\n");
}
#endif

static int aob_stream_lc3_encode_bits(void *lc3_enc_info, void *input_samples,
    void *output_bytes, int32_t num_bytes)
{
    LOG_D("[lc3][enc]\n");
    int err = LC3_API_OK;
    LC3_Enc_Info *p_lc3_enc_info = (LC3_Enc_Info *)lc3_enc_info;
    ASSERT(p_lc3_enc_info, "p_lc3_enc_info is empty in %s ", __func__);
#if AOB_CALCULATE_CODEC_MIPS
    uint32_t cpu_freq = 0;
    uint32_t stime = 0, etime = 0;
    uint32_t use_time_in_us = 0;
    stime = hal_sys_timer_get();

    if (!aob_encode_time_info.codec_started)
    {
        aob_encode_time_info.codec_started = true;
        aob_encode_time_info.total_time_in_us = 0;
        aob_encode_time_info.start_time_in_ms = hal_sys_timer_get();
    }
#endif

#ifdef AOB_CODEC_CP
    GAF_CODEC_CP_PROCESSING_TYPE_E sampleBits = GAF_CODEC_CP_PROCESSING_TYPE_ENCODE;
    if(p_lc3_enc_info->bitwidth==24)
        sampleBits = GAF_CODEC_CP_PROCESSING_TYPE_ENCODE;
    err = gaf_codec_audio_encode_over_cp(sampleBits,
        (uint32_t)p_lc3_enc_info->cb_encode_interlaced, p_lc3_enc_info, p_lc3_enc_info->scratch,
        input_samples, output_bytes, num_bytes);
#else
    err = p_lc3_enc_info->cb_encode_interlaced(p_lc3_enc_info, p_lc3_enc_info->scratch,
        input_samples, output_bytes, num_bytes);
#endif

#if AOB_CALCULATE_CODEC_MIPS
    etime = hal_sys_timer_get();
    use_time_in_us = TICKS_TO_US(etime - stime);
    aob_encode_time_info.total_time_in_us += use_time_in_us;
    aob_decode_time_info.total_etime_in_us += p_lc3_enc_info->frame_dms * 100;
    cpu_freq = aob_lc3_get_sysfreq();
    aob_encode_time_info.codec_mips = cpu_freq *
        aob_encode_time_info.total_time_in_us / aob_decode_time_info.total_etime_in_us;
    LOG_I("err %d ticks:%d time:%d us", err, (etime - stime), use_time_in_us);
    LOG_I("freq %d use:%d ms total:%d ms mips: %d M", cpu_freq,
        aob_encode_time_info.total_time_in_us/1000, aob_decode_time_info.total_etime_in_us/1000,
        aob_encode_time_info.codec_mips);
#endif
    if(err){
        LOG_E("lc3 enc err:%d\n",err);
    }
    LOG_D("[lc3][enc]%d->%d\n",p_lc3_enc_info->frame_samples,p_lc3_enc_info->frame_size);
    return (int)err;
}

static void gaf_audio_lc3_decoder_buf_init(void* _pStreamEnv, uint8_t alg_context_cnt)
{
    LOG_D("%s", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    for (uint8_t i = 0; i < alg_context_cnt; i++) {
        LC3_Dec_Info* info =
            (LC3_Dec_Info *)pStreamEnv->stream_context.codec_alg_context[i].lc3_codec_context.lc3_dec_info;
        info->cb_alloc = lc3_alloc;
        info->cb_free = lc3_free;
        info->cb_overlay = lc3_overlay;
    }
}

#ifdef GAF_DECODER_CROSS_CORE_USE_M55
/**
 ****************************************************************************************
 * @brief bth send m55 init parameters and signal to m55, and finish m55 init phase
 *
 *
 * @param[in] alg_context_cnt              context type count value
 * @param[in] pStreamEnv                   stream env
 *
 * @param[out] NONE                         return NONE
 ****************************************************************************************
 */
static void gaf_audio_lc3_bth_feed_decoder_init_to_m55(uint8_t alg_context_cnt, GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    LOG_I("[%s] start", __func__);
    /* Global variable initialization, including mainly lastReceivedNumb, pcmPacketEntryCount */
    gaf_decoder_core_reset();

    /* NOTE: Can not overlay LC3, because lc3 works with call algo together. */
    // app_overlay_subsys_select(APP_OVERLAY_M55, APP_OVERLAY_SUBSYS_AOB_LC3);

    /* parameters initializtion assignment of lc3 codec */
    LC3_Dec_Info *p_lc3_dec_info =(LC3_Dec_Info *)(pStreamEnv->stream_context.codec_alg_context[alg_context_cnt].lc3_codec_context.lc3_dec_info);

    p_lc3_dec_info->bitrate     = 0;
    p_lc3_dec_info->sample_rate = pStreamEnv->stream_info.playbackInfo.sample_rate;
    p_lc3_dec_info->channels    = pStreamEnv->stream_info.playbackInfo.num_channels;
    p_lc3_dec_info->bitwidth    = pStreamEnv->stream_info.playbackInfo.bits_depth;
    p_lc3_dec_info->frame_dms   = (uint8_t)(pStreamEnv->stream_info.playbackInfo.frame_ms * 10);
    p_lc3_dec_info->frame_size  = pStreamEnv->stream_info.playbackInfo.encoded_frame_size*p_lc3_dec_info->channels;

    /// get maxPcmsizePerFrame value, it is equal to "ch_count * config->frame_samples*((((config->bits_depth + 15)/16)*16)/8)"
    uint32_t nSamples =(uint32_t)(pStreamEnv->stream_info.playbackInfo.sample_rate * pStreamEnv->stream_info.playbackInfo.frame_ms / 1000);

    gaf_decoder_cc_mcu_env.maxPCMSizePerFrame = p_lc3_dec_info->channels * nSamples * ((((p_lc3_dec_info->bitwidth + 15)/16) * 16) / 8);

    /* bth start to send m55 init signal to m55 */
    GAF_DECODER_INIT_REQ_T initReq;
    initReq.stream_cfg.num_channels   = p_lc3_dec_info->channels;
    initReq.stream_cfg.bits_depth     = p_lc3_dec_info->bitwidth;
    initReq.stream_cfg.bitrate        = p_lc3_dec_info->bitrate;
    initReq.stream_cfg.sample_rate    = p_lc3_dec_info->sample_rate;
    initReq.stream_cfg.frame_ms       = p_lc3_dec_info->frame_dms;
    initReq.stream_cfg.frame_size     = p_lc3_dec_info->frame_size;
    initReq.maxPcmDataSizePerFrame    = gaf_decoder_cc_mcu_env.maxPCMSizePerFrame;

    /// According to this ase stream type to decide capture status, and help m55 finish MIPS choosing_task
    initReq.stream_playback_state = false;
    initReq.stream_capture_state  = false;

    if (is_playback_state == true){
        initReq.stream_playback_state = true;
    }

    if (is_capture_state == true){
        initReq.stream_capture_state  = true;
    }

    /// help to judge this device is mobile or buds
    if (is_support_ble_audio_mobile_m55_decode == true)
    {
        initReq.is_support_audio_mobile = true;
    }
    else
    {
        initReq.is_support_audio_mobile = false;
    }

    /// send init req cmd to m55 ask to decoder init
    app_dsp_m55_bridge_send_cmd(
                    CROSS_CORE_INSTANT_CMD_GAF_DECODE_INIT_WAITING_RSP,
                    (uint8_t*)&initReq,
                    sizeof(GAF_DECODER_INIT_REQ_T));

    /* print these lc3 codec parameters */
    LOG_D("Sample rate:           %i", p_lc3_dec_info->sample_rate);
    LOG_D("Channels:              %i", p_lc3_dec_info->channels);
    LOG_D("bitwidth:              %i", p_lc3_dec_info->bitwidth);
    LOG_D("bitrate:               %i", p_lc3_dec_info->bitrate);
    LOG_D("is central role:       %i", initReq.is_support_audio_mobile);
    LOG_D("encoded_data_len:      %i", p_lc3_dec_info->frame_size);
    LOG_I("[%s] end", __func__);

    return ;
}
#endif /// GAF_DECODER_CROSS_CORE_USE_M55

static void gaf_audio_lc3_decoder_init(void* _pStreamEnv, uint8_t alg_context_cnt)
{
    LOG_I("%s", __func__);

#ifdef AOB_CODEC_CP
    gaf_audio_lc3_start_cp_process();
#endif

    /* m55 core decoder init, m55_user is APP_DSP_M55_USER_AUDIO_DECODER */
#ifdef GAF_CODEC_CROSS_CORE
#ifdef DSP_M55
    app_dsp_m55_init(APP_DSP_M55_USER_AUDIO_DECODER);
#endif
#endif

    /* lc3 deocder init phase starting */
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    for (uint8_t i = 0; i < alg_context_cnt; i++) {
#ifdef GAF_DECODER_CROSS_CORE_USE_M55
        gaf_audio_lc3_bth_feed_decoder_init_to_m55(i, pStreamEnv);
#else
        LC3_Dec_Info *p_lc3_dec_info =
            (LC3_Dec_Info *)(pStreamEnv->stream_context.codec_alg_context[i].lc3_codec_context.lc3_dec_info);
        p_lc3_dec_info->sample_rate = pStreamEnv->stream_info.playbackInfo.sample_rate;
        p_lc3_dec_info->channels   = pStreamEnv->stream_info.playbackInfo.num_channels;
        p_lc3_dec_info->bitwidth   = pStreamEnv->stream_info.playbackInfo.bits_depth;
        p_lc3_dec_info->bitalign   = (p_lc3_dec_info->bitwidth==24)?32:0;
        p_lc3_dec_info->frame_dms   = (uint8_t)(pStreamEnv->stream_info.playbackInfo.frame_ms*10);
        p_lc3_dec_info->frame_size = pStreamEnv->stream_info.playbackInfo.encoded_frame_size*p_lc3_dec_info->channels;
        p_lc3_dec_info->bitrate    = 0;
        p_lc3_dec_info->plcMeth    = LC3_API_PLC_ADVANCED;
        p_lc3_dec_info->epmode     = LC3_API_EP_OFF;
        p_lc3_dec_info->is_interlaced     = 1;
        aob_stream_lc3_decoder_init(p_lc3_dec_info);

#if defined(GAF_LC3_BES_PLC_ON)
        lc3_packeet_len = p_lc3_dec_info->sample_rate * p_lc3_dec_info->frame_dms / 10000 * 2;
        lc3_smooth_len = lc3_packeet_len * 4;
        if(p_lc3_dec_info->bitwidth == 16){
            plc_is_16bit = 1;
        }else{
            plc_is_16bit = 0;
        }
        lc3_plc_state1 = (PLC_State *)lc3_alloc(0, sizeof(PLC_State));
        a2dp_plc_lc3_init(lc3_plc_state1, A2DP_PLC_CODEC_TYPE_LC3, lc3_packeet_len / 2);
        if(p_lc3_dec_info->channels == 2)
        {
            lc3_plc_state2 = (PLC_State *)lc3_alloc(0, sizeof(PLC_State));
            a2dp_plc_lc3_init(lc3_plc_state2, A2DP_PLC_CODEC_TYPE_LC3, lc3_packeet_len / 2);
        }
        lc3_cos_buf = (float *)lc3_alloc(0, lc3_smooth_len * sizeof(float));
        cos_generate(lc3_cos_buf, lc3_smooth_len, lc3_packeet_len);
#elif defined(GAF_LC3_MUSIC_PLC_ON)
        memset(&lc3_plc_param,0,sizeof(lc3_plc_param));
        lc3_plc_param.fsHz = p_lc3_dec_info->sample_rate;
        lc3_plc_param.channels = p_lc3_dec_info->channels;
        lc3_plc_param.frameSamples = p_lc3_dec_info->sample_rate * p_lc3_dec_info->frame_dms / 10000;
        lc3_plc_param.sampleFormat.validBits = p_lc3_dec_info->bitwidth;
        lc3_plc_param.sampleFormat.storedBits = p_lc3_dec_info->bitalign;
        lc3_plc_param.sampleFormat.alignMode = PlcSampleHighByteAlign;
        // if(p_lc3_dec_info->bitwidth==24)
        //     lc3_plc_param.sampleFormat.alignMode = PlcSampleLowByteAlign;
        lc3_plc_param.mode = PlcModeMusicPlc;
        lc3_plc_param.MusicPlcParam.overlapMs = MUSIC_PLC_DELAY;
        lc3_plc_param.MusicPlcParam.decayTimeMs = 40;

        int32_t lc3_plc_state_size = GetPlcStateSize(&lc3_plc_param);
        if(lc3_plc_state_size<1)
        {
            TRACE(0, "[%s] GAF: LC3 GetPlcStateSize error, size:%d", __func__,lc3_plc_state_size);
        }
        lc3_plc_state = p_lc3_dec_info->cb_alloc(0, lc3_plc_state_size);
        if(!lc3_plc_state)
        {
            TRACE(0, "[%s] GAF: LC3 alloc error, size:%d", __func__,lc3_plc_state_size);
        }
        PlcRet ret = PlcInit(lc3_plc_state, &lc3_plc_param);
        if(ret!=PLC_RET_SUCCESS)
        {
            TRACE(0, "[%s] GAF: LC3 PlcInit error, ret:%d", __func__,ret);
            TRACE(0, "[%s] GAF: LC3 Plc Init param, fs:%d, ch:%d, fSam:%d, vB:%d, sB:%d, aM:%d, Md:%d, ol:%d, dT:%d", 
            __func__,
            lc3_plc_param.fsHz,lc3_plc_param.channels,lc3_plc_param.frameSamples,
            lc3_plc_param.sampleFormat.validBits,lc3_plc_param.sampleFormat.storedBits,lc3_plc_param.sampleFormat.alignMode,
            lc3_plc_param.mode,lc3_plc_param.MusicPlcParam.overlapMs,lc3_plc_param.MusicPlcParam.decayTimeMs);
        }
        TRACE(0, "[%s] GAF: LC3 Plc Init success", __func__);
#endif
#endif
    }

    /* lc3 audio dump parameters init */
#if (LC3_AUDIO_DOWNLOAD_DUMP)
    uint32_t num_channels = pStreamEnv->stream_info.playbackInfo.num_channels;
    uint32_t dump_frame_len = (uint32_t)(pStreamEnv->stream_info.playbackInfo.sample_rate*pStreamEnv->stream_info.playbackInfo.frame_ms/1000);
    auto bitalign = (pStreamEnv->stream_info.playbackInfo.bits_depth == 24)? 32 : pStreamEnv->stream_info.playbackInfo.bits_depth;
    audio_dump_init(dump_frame_len, bitalign/8, num_channels);
    TRACE(0, "dump_frame_len:%d, bitalign:%d", dump_frame_len, bitalign);
#endif

    LOG_I("%s end", __func__);
}

static void gaf_audio_lc3_decoder_deinit(void)
{
    app_overlay_unloadall();
#ifdef AOB_CODEC_CP
    gaf_audio_lc3_stop_cp_process();
#endif
    LOG_I("%s", __FUNCTION__);
}

POSSIBLY_UNUSED static int gaf_audio_lc3_decode(bool isPLC, uint32_t inputDataLength, void* input,
    gaf_codec_algorithm_context_t *algo_context, void* output)
{
    //GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    int ret = LC3_API_ERROR;
    ret = aob_stream_lc3_decode_bits(
        algo_context->lc3_codec_context.lc3_dec_info, input,
        inputDataLength, output, isPLC);
 #if LC3_AUDIO_DOWNLOAD_DUMP
    LC3_Dec_Info *p_lc3_dec_info = (LC3_Dec_Info *)algo_context->lc3_codec_context.lc3_dec_info;
    auto frame_samples = p_lc3_dec_info->frame_samples;
    //LOG_I("[output_len] %d", frame_samples);
    audio_dump_clear_up();
    audio_dump_add_channel_data(0, output, frame_samples);
    audio_dump_run();
#endif

    return ret;
}

POSSIBLY_UNUSED static int gaf_audio_lc3_decode_two_channel(bool isPLC, uint32_t inputDataLength, void* input,
    gaf_codec_algorithm_context_t *algo_context, void* output)
{
    POSSIBLY_UNUSED LC3_Dec_Info *lc3_dec_info = (LC3_Dec_Info *)algo_context->lc3_codec_context.lc3_dec_info;
    //GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    int ret = LC3_API_ERROR;
    void* pcms[2];
    pcms[0] = output;
    pcms[1] = (uint8_t*)output + (lc3_dec_info->bitwidth)/8;
    ret = aob_stream_lc3_decode_bits(
        algo_context->lc3_codec_context.lc3_dec_info, input,
        inputDataLength, pcms, isPLC);

#if LC3_AUDIO_DOWNLOAD_DUMP
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    uint8_t cis_channel_0 = pStreamEnv->stream_context.playback_buff_list[0].cisChannel;
    uint8_t cis_channel_1 = pStreamEnv->stream_context.playback_buff_list[1].cisChannel;
    LOG_I("cis_channel_0 %d  cis_channel_1 %d ", cis_channel_0,cis_channel_1);
    if (ble_audio_is_ux_mobile() && cis_channel_0 == 3)
    {
        uint16_t *input_samples_16[GAF_AUDIO_MAX_CHANNELS] = {NULL};
        uint16_t *input_buf_16 = (uint16_t *)output;
        input_samples_16[0] = (uint16_t *)gaf_stream_heap_malloc(inputDataLength*4);
        input_samples_16[1] = (uint16_t *)gaf_stream_heap_malloc(inputDataLength*4);
        for (uint32_t i=0;i < (inputDataLength*2);i++)
        {
            input_samples_16[0][i] = input_buf_16[i*2];
            input_samples_16[1][i] = input_buf_16[i*2+1];
        }
        uint32_t output_len =
        (uint32_t)(lc3_dec_info->sample_rate*lc3_dec_info->frame_dms/1000);
        LOG_I("[output_len] %d  inputDataLength %d ", output_len,inputDataLength);
        audio_dump_clear_up();

        //audio_dump_add_channel_data(0, output, output_len);
        for(uint8_t i = 0; i < lc3_dec_info->channels; i++)
        {
            audio_dump_add_channel_data(i, input_samples_16[i], output_len);
        }

        audio_dump_run();

        gaf_stream_heap_free(input_samples_16[0]);
        gaf_stream_heap_free(input_samples_16[1]);
        }
#endif

    return ret;
}

static void gaf_audio_lc3_decoder_buf_deinit(void* _pStreamEnv, uint8_t alg_context_cnt)
{
#ifdef AOB_CODEC_CP
    gaf_codec_cp_heap_deinit();
#endif

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    for (uint8_t i = 0; i < alg_context_cnt; i++) {
        LC3_Dec_Info *p_lc3_dec_info =
            (LC3_Dec_Info *)pStreamEnv->stream_context.codec_alg_context[i].lc3_codec_context.lc3_dec_info;
        if(p_lc3_dec_info->cb_uninit)
            p_lc3_dec_info->cb_uninit(p_lc3_dec_info);
        lc3_api_decoder_extra_buf_deinit(p_lc3_dec_info);
        p_lc3_dec_info->instance = NULL;
        p_lc3_dec_info->scratch = NULL;
        if(p_lc3_dec_info->enabled_frame_buff)
            lc3_free(0,p_lc3_dec_info->enabled_frame_buff);
#if defined(GAF_LC3_MUSIC_PLC_ON)
        if(lc3_plc_state != NULL)
            p_lc3_dec_info->cb_free(0, lc3_plc_state);
#endif
    }
}

static const GAF_AUDIO_DECODER_FUNC_LIST_T gaf_audio_lc3_decoder_func_list =
{
    .decoder_init_buf_func = gaf_audio_lc3_decoder_buf_init,
    .decoder_init_func = gaf_audio_lc3_decoder_init,
    .decoder_deinit_func = gaf_audio_lc3_decoder_deinit,
    .decoder_decode_frame_func = gaf_audio_lc3_decode,
    .decoder_deinit_buf_func = gaf_audio_lc3_decoder_buf_deinit,
};

static void gaf_audio_lc3_encoder_buf_init(void* _pStreamEnv)
{
    TRACE(1, "%s", __func__);
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
/*
*   Capture should always be less than 1
*/
    LC3_Enc_Info* info =
        (LC3_Enc_Info *)pStreamEnv->stream_context.codec_alg_context[0].lc3_codec_context.lc3_enc_info;
#if defined(AOB_CODEC_CP) && !defined(UNIFY_HEAP_ENABLED)
    gaf_codec_cp_heap_init();
#endif
    info->cb_alloc = lc3_alloc;
    info->cb_free = lc3_free;
    info->cb_overlay = lc3_overlay;
}
#include "app_utils.h"

#ifdef GAF_ENCODER_CROSS_CORE_USE_M55
/**
 ****************************************************************************************
 * @brief bth send m55 encoder init parameters and signal to m55, and finish m55 init phase
 *
 *
 *
 * @param[in] pStreamEnv                   stream env
 *
 * @param[out] NONE                         return NONE
 ****************************************************************************************
 */
static void gaf_audio_lc3_bth_feed_encoder_init_to_m55(GAF_AUDIO_STREAM_ENV_T* pStreamEnv)
{
    /* Global variable initialization, including mainly lastReceivedNumb, pcmPacketEntryCount */
    gaf_encoder_core_reset();

    /* NOTE: Can not overlay LC3, because lc3 works with call algo together. */
    // app_overlay_subsys_select(APP_OVERLAY_M55, APP_OVERLAY_SUBSYS_AOB_LC3);

    /* parameters initializtion assignment of lc3 codec */

    LC3_Enc_Info *p_lc3_enc_info =
        (LC3_Enc_Info *)(pStreamEnv->stream_context.codec_alg_context[0].lc3_codec_context.lc3_enc_info);

    p_lc3_enc_info->bitrate     = 0;
    p_lc3_enc_info->sample_rate = pStreamEnv->stream_info.captureInfo.sample_rate;
    p_lc3_enc_info->channels    = pStreamEnv->stream_info.captureInfo.num_channels;
    p_lc3_enc_info->bitwidth    = pStreamEnv->stream_info.captureInfo.bits_depth;
    p_lc3_enc_info->frame_dms   = (uint8_t)(pStreamEnv->stream_info.captureInfo.frame_ms*10.f);
    p_lc3_enc_info->frame_size  = pStreamEnv->stream_info.captureInfo.encoded_frame_size*p_lc3_enc_info->channels;

    /// get max perFrame encoded packet size
    gaf_encoder_cc_mcu_env.maxEncodeDataSizePerFrame = gaf_audio_lc3_encoder_get_max_frame_size();

    /* bth send encoder init signal to m55 core, and ask to m55 encoder init */

    GAF_ENCODER_INIT_REQ_T initReq;

    initReq.stream_playback_state = false;
    initReq.stream_capture_state  = false;

    /// help m55 core decide MIPS's design
    if (is_playback_state == true){
        initReq.stream_playback_state = true;
    }

    if (is_capture_state == true){
        initReq.stream_capture_state  = true;
    }

    initReq.stream_cfg.channels         = p_lc3_enc_info->channels;
    initReq.stream_cfg.bitwidth         = p_lc3_enc_info->bitwidth;
    initReq.stream_cfg.bitrate          = p_lc3_enc_info->bitrate;
    initReq.stream_cfg.sample_rate      = p_lc3_enc_info->sample_rate;
    initReq.stream_cfg.frame_dms        = p_lc3_enc_info->frame_dms;
    initReq.stream_cfg.frame_size       = p_lc3_enc_info->frame_size;
    initReq.maxEncodedDataSizePerFrame  = gaf_audio_lc3_encoder_get_max_frame_size();

    initReq.tx_algo_cfg = pStreamEnv->stream_info.tx_algo_cfg;

    /// send encoded init cmd to m55 core
    app_dsp_m55_bridge_send_cmd(
                    CROSS_CORE_TASK_CMD_GAF_ENCODE_INIT_WAITING_RSP, \
                    (uint8_t*)&initReq, \
                    sizeof(GAF_ENCODER_INIT_REQ_T));

}
#endif

static void gaf_audio_lc3_encoder_init(void* _pStreamEnv)
{
    LOG_I("%s", __func__);

#ifdef AOB_CODEC_CP
    gaf_audio_lc3_start_cp_process();
#endif

    /* m55 core decoder init, m55_user is APP_DSP_M55_USER_AUDIO_DECODER */
#ifdef GAF_CODEC_CROSS_CORE
#ifdef DSP_M55
        app_dsp_m55_init(APP_DSP_M55_USER_AUDIO_ENCODER);
#endif
#endif

    //Capture should always be less than 1
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

#if defined (GAF_ENCODER_CROSS_CORE_USE_M55)
    gaf_audio_lc3_bth_feed_encoder_init_to_m55(pStreamEnv);
#else
    LC3_Enc_Info *p_lc3_enc_info =
        (LC3_Enc_Info *)(pStreamEnv->stream_context.codec_alg_context[0].lc3_codec_context.lc3_enc_info);

    p_lc3_enc_info->bitrate       = 0;
    p_lc3_enc_info->is_interlaced = 1;
    p_lc3_enc_info->epmode        = LC3_API_EP_OFF;
    p_lc3_enc_info->ltpf_flag     = 1;
    p_lc3_enc_info->quan_prec     = 48;
    p_lc3_enc_info->sample_rate   = pStreamEnv->stream_info.captureInfo.sample_rate;
    p_lc3_enc_info->channels      = pStreamEnv->stream_info.captureInfo.num_channels;
    p_lc3_enc_info->bitwidth      = pStreamEnv->stream_info.captureInfo.bits_depth;
    p_lc3_enc_info->bitalign      = (p_lc3_enc_info->bitwidth==24)?32:p_lc3_enc_info->bitwidth;
    p_lc3_enc_info->frame_dms     = (uint8_t)(pStreamEnv->stream_info.captureInfo.frame_ms*10.f);
    p_lc3_enc_info->frame_size    = pStreamEnv->stream_info.captureInfo.encoded_frame_size*p_lc3_enc_info->channels;
    aob_stream_lc3_encoder_init(p_lc3_enc_info);
#endif

#if LC3_AUDIO_UPLOAD_DUMP
    uint32_t dump_frame_len = (uint32_t)(p_lc3_enc_info->sample_rate*p_lc3_enc_info->frame_dms/10000);
    audio_dump_init(dump_frame_len, p_lc3_enc_info->bitalign/8, 1);
#endif
}

static void gaf_audio_lc3_encoder_deinit(void* _pStreamEnv)
{
#ifdef AOB_CODEC_CP
    gaf_audio_lc3_stop_cp_process();
#endif

    LOG_I("%s", __FUNCTION__);
}

static void gaf_audio_lc3_encode(void* _pStreamEnv, uint32_t timeStamp,
    uint32_t inputDataLength, void *input, gaf_codec_algorithm_context_t *algo_context,
        GAF_AUDIO_ENCODER_SEND_FRAME_FUNC cbsend)
{
    //GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    LC3_Enc_Info *lc3_enc_info = (LC3_Enc_Info *)algo_context->lc3_codec_context.lc3_enc_info;
    uint32_t lc3_encoded_frame_len = lc3_enc_info->frame_size;
    #if LC3_AUDIO_UPLOAD_DUMP
        auto  frame_samples = lc3_enc_info->frame_samples;
        char* input_ptr = (char*)input;
        auto sample_len = lc3_enc_info->bitalign/8*frame_samples;
        auto chs = lc3_enc_info->channels;
        for(int ch = 0; ch < chs; ch++){
            audio_dump_clear_up();
            audio_dump_add_channel_data(0, input_ptr+ch*sample_len, frame_samples);
            audio_dump_run();
        }
    #endif
        void* output_bytes = lc3_enc_info->enabled_frame_buff;
        if(!output_bytes){
            LOG_E("[lc3][err]output buff is NULL");
            return;
        }
        aob_stream_lc3_encode_bits(lc3_enc_info, input, output_bytes, lc3_encoded_frame_len);

    if(cbsend){
        cbsend(_pStreamEnv,output_bytes,lc3_enc_info->frame_size,timeStamp);
    }
}

static void gaf_audio_lc3_encoder_buf_deinit(void* _pStreamEnv)
{
#ifdef AOB_CODEC_CP
    gaf_codec_cp_heap_deinit();
#endif

    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    //Capture should always be less than 1
    LC3_Enc_Info *p_lc3_enc_info =
        (LC3_Enc_Info *)pStreamEnv->stream_context.codec_alg_context[0].lc3_codec_context.lc3_enc_info;
    if(p_lc3_enc_info->cb_uninit)
        p_lc3_enc_info->cb_uninit(p_lc3_enc_info);
    lc3_api_encoder_extra_buf_deinit(p_lc3_enc_info);
    p_lc3_enc_info->instance= NULL;
    p_lc3_enc_info->scratch= NULL;
}

const GAF_AUDIO_ENCODER_FUNC_LIST_T gaf_audio_lc3_encoder_func_list =
{
    .encoder_init_buf_func = gaf_audio_lc3_encoder_buf_init,
    .encoder_init_func = gaf_audio_lc3_encoder_init,
    .encoder_deinit_func = gaf_audio_lc3_encoder_deinit,
    .encoder_encode_frame_func = gaf_audio_lc3_encode,
    .encoder_deinit_buf_func = gaf_audio_lc3_encoder_buf_deinit,
};


///gaf audio lc3 interface
void gaf_audio_lc3_update_codec_func_list(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;

    pStreamEnv->func_list->decoder_func_list = &gaf_audio_lc3_decoder_func_list;
    pStreamEnv->func_list->encoder_func_list = &gaf_audio_lc3_encoder_func_list;
    pStreamEnv->stream_info.codec = "lc3";
}

#ifdef LC3PLUS_SUPPORT
void gaf_audio_lc3plus_update_codec_func_list(void* _pStreamEnv)
{
    GAF_AUDIO_STREAM_ENV_T* pStreamEnv = (GAF_AUDIO_STREAM_ENV_T *)_pStreamEnv;
    //TODO:change lc3 to lc3plus
    pStreamEnv->func_list->decoder_func_list = &gaf_audio_lc3_decoder_func_list;
    pStreamEnv->func_list->encoder_func_list = &gaf_audio_lc3_encoder_func_list;
    pStreamEnv->stream_info.codec = "lc3plus";
}
#endif

int gaf_audio_lc3_encoder_get_max_frame_size(void){
    return LC3_MAX_FRAME_SIZE;
}

#endif

#ifdef BLE_SMF
enum APP_SYSFREQ_FREQ_T gaf_audio_stream_encoder_adjust_bth_freq_no_m55(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_)
{
    enum APP_SYSFREQ_FREQ_T gaf_audio_m55_base_freq = _base_freq_;
    if (frame_ms == 0)
    {
        ASSERT(0, "%s, wrong Divided ", __func__);
    }
    uint32_t lc3_bitrate = frame_size * 80000 / frame_ms;

    if (44100 == sample_rate){
        lc3_bitrate = lc3_bitrate * 44100 / 48000;
    }
    LOG_I("bth_encoder_lc3_bitrate:%d sample_rate: %d", lc3_bitrate, sample_rate);

    if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_52M;
    }
    else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_104M;
    }
    else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_52M;
    }
    else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
    }
    else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
    }
    else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_78M;
    }
    else if ((64000 == lc3_bitrate) && (48000 == sample_rate)){
        gaf_audio_m55_base_freq = APP_SYSFREQ_104M;
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
    if (channel >= 2)
    {
#ifdef HAL_CMU_FREQ_15M
        gaf_audio_m55_base_freq = APP_SYSFREQ_144M;
#else
        gaf_audio_m55_base_freq = APP_SYSFREQ_208M;
#endif
    }
    return gaf_audio_m55_base_freq;
}

enum APP_SYSFREQ_FREQ_T gaf_audio_flexible_decoder_adjust_bth_freq_no_m55(uint16_t frame_size, uint8_t channel,
    uint8_t frame_ms, uint32_t sample_rate, enum APP_SYSFREQ_FREQ_T _base_freq_)
{
    enum APP_SYSFREQ_FREQ_T gaf_audio_base_freq = _base_freq_;
    uint32_t lc3_bitrate = frame_size * 80000 / frame_ms;

    if (44100 == sample_rate){
        lc3_bitrate = lc3_bitrate * 44100 / 48000;
    }
    LOG_I("LC3_bitrate:%d", lc3_bitrate);

    if ((16000 == lc3_bitrate) && (8000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_12M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    #endif
    }
    else if ((16000 == lc3_bitrate) && (16000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_15M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    #endif
    }
    else if ((32000 == lc3_bitrate) && (16000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_15M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    #endif
    }
    else if ((32000 == lc3_bitrate) && (32000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_15M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    #endif
    }
    else if ((48000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    }
    else if ((64000 == lc3_bitrate) && (32000 == sample_rate)){
        gaf_audio_base_freq = APP_SYSFREQ_26M;
    }
    else if ((88200 == lc3_bitrate) && (44100 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_30M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_52M;
    #endif
    }
    else if ((96000 == lc3_bitrate) && (48000 == sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_30M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_52M;
    #endif
    }
    else if ((96000 < lc3_bitrate) && (48000 < sample_rate)){
    #ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_48M;
    #else
        gaf_audio_base_freq = APP_SYSFREQ_52M;
    #endif
    }
    if (channel >= 2)
    {
#ifdef HAL_CMU_FREQ_15M
        gaf_audio_base_freq = APP_SYSFREQ_144M;
#else
        gaf_audio_base_freq = APP_SYSFREQ_208M;
#endif
    }
    return gaf_audio_base_freq;
}

void lc3_alloc_data_free(void)
{
    for (uint32_t i = 0; i < GAF_CODEC_LC3_BUF_ENTER_CONT; i++) {
        lc3_buf_entry[i].is_used = false;
        lc3_buf_entry[i].size = 0;
        lc3_buf_entry[i].data = NULL;
    }
}

#endif