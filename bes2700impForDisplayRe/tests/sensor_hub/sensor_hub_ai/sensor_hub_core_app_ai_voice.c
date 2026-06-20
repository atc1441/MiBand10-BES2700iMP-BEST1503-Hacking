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

#ifdef VOICE_DETECTOR_EN
#include "string.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hwtimer_list.h"
#include "analog.h"
#include "audioflinger.h"
#include "cqueue.h"
#include "sens_msg.h"
#include "sensor_hub_core.h"
#include "sensor_hub_core_app.h"
#include "sensor_hub_vad_core.h"
#include "sensor_hub_vad_adapter.h"
#include "app_voice_detector.h"
#include "sensor_hub_core_app_ai.h"
#include "sensor_hub_core_app_ai_voice.h"

#ifdef THIRDPARTY_LIB_SS
#include "ssvt_stream.h"
#endif
#ifdef THIRDPARTY_LIB_GG
#include "gsound_lib.h"
#endif
#ifdef THIRDPARTY_LIB_ALEXA
#include "alexa_lib.h"
#endif

//#define SENSOR_HUB_AI_AUDIO_DUMP_EN
#ifdef SENSOR_HUB_AI_AUDIO_DUMP_EN
#include "audio_dump.h"
#endif

/*
 * capture stream initilizations
 **************************************************************
 */
#define ALIGNED4                        ALIGNED(4)
#define NON_EXP_ALIGN(val, exp)         (((val) + ((exp) - 1)) / (exp) * (exp))
#define AF_DMA_LIST_CNT                 4
#define CHAN_NUM_CAPTURE                1
#define RX_BURST_NUM                    4
#define RX_ALL_CH_DMA_BURST_SIZE        (RX_BURST_NUM * 2 * CHAN_NUM_CAPTURE)
#define RX_BUFF_ALIGN                   (RX_ALL_CH_DMA_BURST_SIZE * AF_DMA_LIST_CNT)

#define CAPTURE_SAMP_RATE               16000 //48000
#define CAPTURE_SAMP_BITS               16
#define CAPTURE_SAMP_SIZE               2
#define CAPTURE_CHAN_NUM                CHAN_NUM_CAPTURE
#define CAPTURE_FRAME_NUM               32
#define CAPTURE_FRAME_SIZE              (CAPTURE_SAMP_RATE / 1000 * CAPTURE_SAMP_SIZE * CHAN_NUM_CAPTURE)
#define CAPTURE_SIZE                    NON_EXP_ALIGN(CAPTURE_FRAME_SIZE * CAPTURE_FRAME_NUM, RX_BUFF_ALIGN)

static void sensor_hub_voice_kws_key_word_handler(void * kws_info,uint16_t len);

static ai_voice_kws_user_instant_t ai_voice_kws_instant[] = 
{
    /* each ai user instant descriptor*/

#ifdef THIRDPARTY_LIB_GG
        /*google*/
        {
            .voice_user = SENSOR_HUB_AI_USER_GG,
            .activate = false,
            .handler =
            {
                .init_handler = gsound_lib_init,
                .voice_kws_shot_frame_info_handler = app_gsound_voice_read_ind_ptr_get,
                .voice_kws_frame_data_handler = gsound_lib_parse,
                .stream_prepare_start_handler = app_gsound_lib_voice_reset,
                .stream_prepare_stop_handler = NULL,
            },
        },
#endif
#ifdef THIRDPARTY_LIB_ALEXA
        /*alexa*/
        {
            .voice_user = SENSOR_HUB_AI_USER_ALEXA,
            .activate = false,
            .handler =
            {
                .init_handler = alexa_kws_wwe_init,
                .voice_kws_shot_frame_info_handler = app_alexa_voice_read_ind_ptr_get,
                .voice_kws_frame_data_handler = alexa_kws_data_process,
                .stream_prepare_start_handler = app_alexa_lib_voice_reset,
                .stream_prepare_stop_handler = NULL,
            },
        },
#endif

#ifdef THIRDPARTY_LIB_SS
        /*ss*/
        {
            .voice_user = SENSOR_HUB_AI_USER_SS,
            .activate = false,
            .handler =
            {
                .init_handler = ssvt_lib_init,
                .voice_kws_shot_frame_info_handler = app_ss_voice_read_ind_ptr_get,
                .voice_kws_frame_data_handler = ssvt_lib_parse_sound,
                .stream_prepare_start_handler = app_ss_lib_voice_reset,
                .stream_prepare_stop_handler = NULL,
            },
        },

#endif
};

static uint8_t app_vad_adpt_kws_infor_payload[KWS_INFOR_PAYLOAD_LEN];
CQueue voice_kws_pcm_queue;
static uint8_t raw_pcm_data_buf_of_queue[KWS_RAW_PCM_DATA_QUEUE_BUF_LEN];
static uint8_t raw_pcm_data_buf_of_per_shot[KWS_RAW_PCM_DATA_BUF_LEN_PER_SHOT];
static uint8_t ALIGNED4 sens_codec_cap_buf[CAPTURE_SIZE];

osMutexId voice_kws_op_mutexID;
osMutexDef(voice_kws_op_mutex);
static osThreadId sensor_hub_kws_tid = NULL;
static void sensor_hub_kws_thread(void const *argument);
osThreadDef(sensor_hub_kws_thread, osPriorityAboveNormal, 1, 1024 * 2, "sensor_hub_kws_thread");

uint32_t sensor_hub_voice_kws_raw_pcm_data_input_signal(uint8_t*buf,uint32_t len)
{
    osMutexWait(voice_kws_op_mutexID, osWaitForever);
    EnCQueue_AI(&voice_kws_pcm_queue, buf, len, NULL);
    osMutexRelease(voice_kws_op_mutexID);
    osSignalSet(sensor_hub_kws_tid, SENSOR_HUB_KWS_SIGNAL_ID);

    return 0;
}

POSSIBLY_UNUSED static uint32_t sensor_hub_voice_kws_pcm_input_queue_length(void)
{
    uint32_t len;
    osMutexWait(voice_kws_op_mutexID, osWaitForever);
    len = LengthOfCQueue(&voice_kws_pcm_queue);
    osMutexRelease(voice_kws_op_mutexID);
    return len;
}

uint32_t sensor_hub_voice_kws_get_cached_len_according_to_read_offset(uint32_t offset)
{
    ASSERT((offset <= KWS_RAW_PCM_DATA_QUEUE_BUF_LEN), 
        "invalid offset %d received.", offset);

    uint32_t cached_len = 0;
    uint32_t write_offset = 0;

    osMutexWait(voice_kws_op_mutexID, osWaitForever);
    write_offset = GetCQueueWriteOffset(&voice_kws_pcm_queue);

    if (write_offset >= offset)
    {
        cached_len = write_offset - offset;
    }
    else
    {
        cached_len = KWS_RAW_PCM_DATA_QUEUE_BUF_LEN - offset + write_offset;
    }
    osMutexRelease(voice_kws_op_mutexID);

    return cached_len;
}


POSSIBLY_UNUSED static void sensor_hub_voice_kws_pcm_input_consume(uint8_t* buf, uint32_t len)
{
    osMutexWait(voice_kws_op_mutexID, osWaitForever);
    DeCQueue(&voice_kws_pcm_queue, buf, len);
    osMutexRelease(voice_kws_op_mutexID);
}

static void sensor_hub_voice_kws_pcm_input_fetch(uint32_t offset, uint8_t *buf, uint32_t len)
{
    osMutexWait(voice_kws_op_mutexID, osWaitForever);
    PeekCQueueToBufWithOffset(&voice_kws_pcm_queue, buf, len, offset);
    osMutexRelease(voice_kws_op_mutexID);
}

static void sensor_hub_voice_kws_pcm_input_seek(uint32_t * ind,uint32_t shot_len)
{
    *ind = (*ind+shot_len) % KWS_RAW_PCM_DATA_QUEUE_BUF_LEN;
}

static void sensor_hub_voice_kws_pcm_input_queue_init(void)
{
    osMutexWait(voice_kws_op_mutexID, osWaitForever);

    memset(raw_pcm_data_buf_of_queue,0,KWS_RAW_PCM_DATA_QUEUE_BUF_LEN);
    memset(raw_pcm_data_buf_of_per_shot,0,KWS_RAW_PCM_DATA_BUF_LEN_PER_SHOT);
    InitCQueue(&voice_kws_pcm_queue, KWS_RAW_PCM_DATA_QUEUE_BUF_LEN, (CQItemType *)raw_pcm_data_buf_of_queue);

    osMutexRelease(voice_kws_op_mutexID);
}

POSSIBLY_UNUSED static void sensor_hub_voice_kws_pcm_input_queue_reset(void)
{
    osMutexWait(voice_kws_op_mutexID, osWaitForever);

    memset(raw_pcm_data_buf_of_queue,0,KWS_RAW_PCM_DATA_QUEUE_BUF_LEN);
    memset(raw_pcm_data_buf_of_per_shot,0,KWS_RAW_PCM_DATA_BUF_LEN_PER_SHOT);
    ResetCQueue(&voice_kws_pcm_queue);

    osMutexRelease(voice_kws_op_mutexID);
}


static void sensor_hub_voice_kws_thread_init(void)
{
    if (voice_kws_op_mutexID== NULL)
    {
        voice_kws_op_mutexID = osMutexCreate((osMutex(voice_kws_op_mutex)));
    }

    sensor_hub_voice_kws_pcm_input_queue_init();

    if (NULL == sensor_hub_kws_tid)
    {
        /// creat AI voice thread
        sensor_hub_kws_tid = osThreadCreate(osThread(sensor_hub_kws_thread), NULL);
    }

    TRACE(3,"%s initial finished %p %p",__func__,voice_kws_op_mutexID,sensor_hub_kws_tid);
#ifdef SENSOR_HUB_AI_AUDIO_DUMP_EN
    audio_dump_init(1536/2, 2, 1);
#endif
}


static void sensor_hub_kws_thread(void const *argument)
{
    osEvent evt;
    while (1)
    {
        evt = osSignalWait(SENSOR_HUB_KWS_SIGNAL_ID, osWaitForever);

        if(evt.status == osEventSignal)
        {
            uint8_t i = 0;
            uint32_t available_len;
            uint32_t shot_len = 0;
            uint32_t * seek_ind;
            voice_kws_shot_get_frame_len_handler_t frame_info_handler;
            voice_kws_data_handler_t data_handler;

            for(i=0; i< ARRAY_SIZE(ai_voice_kws_instant);i++){

                if(!(ai_voice_kws_instant[i].activate)){
                    continue;
                }

                frame_info_handler = ai_voice_kws_instant[i].handler.voice_kws_shot_frame_info_handler;
                data_handler = ai_voice_kws_instant[i].handler.voice_kws_frame_data_handler;

                if(frame_info_handler == NULL){
                    continue;
                }

                shot_len = frame_info_handler(&seek_ind);
                available_len = sensor_hub_voice_kws_get_cached_len_according_to_read_offset(*seek_ind);
                if(available_len < shot_len){
                    continue;
                }
                sensor_hub_voice_kws_pcm_input_fetch(*seek_ind,raw_pcm_data_buf_of_per_shot,shot_len);
                sensor_hub_voice_kws_pcm_input_seek(seek_ind,shot_len);

                if(data_handler == NULL){
                    continue;
                }
#ifdef SENSOR_HUB_AI_AUDIO_DUMP_EN
                audio_dump_add_channel_data(0,raw_pcm_data_buf_of_per_shot,shot_len/2);
                audio_dump_run();
#endif

                data_handler(raw_pcm_data_buf_of_per_shot,shot_len);
            }
        }
    }
}

static inline uint8_t sensor_hub_voice_kws_search_ai_user(SENSOR_HUB_AI_USER_E user)
{
    uint8_t i =0;
    for(i = 0;i<ARRAY_SIZE(ai_voice_kws_instant);i++){
        if(ai_voice_kws_instant[i].voice_user == user){
            break;
        }
    }

    return i;
}

bool sensor_hub_voice_kws_activate_op(SENSOR_HUB_AI_USER_E user, bool activate)
{
    uint8_t index = sensor_hub_voice_kws_search_ai_user(user);
    bool nRet = false;

    if(index != ARRAY_SIZE(ai_voice_kws_instant)){
        ai_voice_kws_instant[index].activate = activate;
        nRet = true;
        if(activate){
            ai_voice_kws_instant[index].handler.init_handler(true,sensor_hub_voice_kws_key_word_handler);
        }
    }
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_13, HAL_CMU_FREQ_52M);
    return nRet;
}

static void sensor_hub_voice_kws_fill_in_kws_env_infor(AI_KWS_INFOR_ENV_INFOR_T * env_info)
{
    uint32_t * src_start_addr;
    uint32_t * src_end_addr;
    uint32_t src_len;
    app_vad_adapter_get_unread_data_infor(&src_start_addr,&src_end_addr,&src_len);

    env_info->history_pcm_data_len = src_len;
}

static void sensor_hub_voice_kws_key_word_handler(void * kws_info,uint16_t len)
{
    TRACE(0, "%s: $$$ len = %d", __func__,len);
    AI_KWS_INFOR_T * kws_infor_ptr = (AI_KWS_INFOR_T *)app_vad_adpt_kws_infor_payload;

    sensor_hub_voice_kws_fill_in_kws_env_infor(&(kws_infor_ptr->env_info));
    memcpy((uint8_t*)&(kws_infor_ptr->kws_infor),(uint8_t*)kws_info,len);

    app_sensor_hub_core_vad_send_evt_msg(SENS_EVT_ID_VAD_VOICE_CMD, 0, 0, 0);
    app_sensor_hub_core_ai_send_msg(SENSOR_HUB_AI_MSG_TYPE_KWS, (void *)kws_infor_ptr,sizeof(AI_KWS_INFOR_T));
}

void sensor_hub_voice_kws_init_capture_stream(struct AF_STREAM_CONFIG_T *cfg)
{
    TRACE(0, "%s:", __func__);

    sensor_hub_voice_kws_thread_init();

    memset(cfg, 0, sizeof(struct AF_STREAM_CONFIG_T));

    //init capture stream
    //adapter will setup default handler for capture stream
    cfg->sample_rate  = CAPTURE_SAMP_RATE;
    cfg->bits         = CAPTURE_SAMP_BITS;
    cfg->channel_num  = CAPTURE_CHAN_NUM;
    cfg->channel_map  = 0;
    cfg->device       = AUD_STREAM_USE_INT_CODEC;
    cfg->vol          = TGT_ADC_VOL_LEVEL_7;
    cfg->handler      = NULL;
    cfg->io_path      = AUD_INPUT_PATH_VADMIC;
    cfg->data_ptr     = sens_codec_cap_buf;
    cfg->data_size    = CAPTURE_SIZE;
}

void sensor_hub_voice_kws_ai_env_setup(uint8_t * buf,uint32_t len)
{
    AI_ENV_SETUP_INFO_T *env_info = (AI_ENV_SETUP_INFO_T *)buf;
    TRACE(2,"%s role = %d ",__func__,env_info->user);
    switch(env_info->user)
    {
        case SENSOR_HUB_AI_USER_GG:
#ifdef THIRDPARTY_LIB_GG
            app_gsound_lib_env_set_up((void*)&(env_info->info));
#endif
            break;
        case SENSOR_HUB_AI_USER_ALEXA:
            break;
        case SENSOR_HUB_AI_USER_SS:
            break;
        default:
            break;
    }
}

void sensor_hub_voice_kws_ai_user_activate(SENSOR_HUB_AI_USER_E role, uint32_t op)
{
    TRACE(3,"%s role = %d op = %d",__func__,role,op);
    switch(role)
    {
        case SENSOR_HUB_AI_USER_BES:
        case SENSOR_HUB_AI_USER_GG:
        case SENSOR_HUB_AI_USER_ALEXA:
        case SENSOR_HUB_AI_USER_SS:
            sensor_hub_voice_kws_activate_op(role,op);
            break;
        default:
            break;
        break;
    }
}

uint32_t sensor_hub_voice_kws_stream_prepare_start(uint8_t *buf, uint32_t len)
{
    uint8_t i = 0;
    for(i=0; i< ARRAY_SIZE(ai_voice_kws_instant);i++){
        if(!(ai_voice_kws_instant[i].activate)){
            continue;
        }
        if(ai_voice_kws_instant[i].handler.stream_prepare_start_handler){
            ai_voice_kws_instant[i].handler.stream_prepare_start_handler();
        }
    }

    sensor_hub_voice_kws_pcm_input_queue_reset();

    sensor_hub_voice_kws_raw_pcm_data_input_signal(buf,len);

    return 0;
}

uint32_t sensor_hub_voice_kws_stream_prepare_stop(uint8_t *buf, uint32_t len)
{
    uint8_t i = 0;
    for(i=0; i< ARRAY_SIZE(ai_voice_kws_instant);i++){
        if(!(ai_voice_kws_instant[i].activate)){
            continue;
        }
        if(ai_voice_kws_instant[i].handler.stream_prepare_stop_handler){
            ai_voice_kws_instant[i].handler.stream_prepare_stop_handler();
        }
    }

    return 0;
}
#endif
