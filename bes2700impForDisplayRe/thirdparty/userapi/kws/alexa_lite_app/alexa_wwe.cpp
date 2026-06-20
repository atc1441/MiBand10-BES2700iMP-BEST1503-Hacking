#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "cmsis.h"
#include "cqueue.h"
#include "bluetooth_bt_api.h"
#include "hal_cmu.h"
#include "hal_aud.h"
#include "hal_timer.h"
#include "hal_sleep.h"
#include "hal_trace.h"
#include "audioflinger.h"
#include "app_utils.h"
#include "audio_dump.h"
#include "app_thirdparty.h"
#include "app_ai_if.h"
#include "app_ai_if_thirdparty.h"
#include "app_ai_voice.h"
#include "pryon_lite_PRL1000.h"
#include "pryon_lite_common_client_properties.h"
#include "ai_thread.h"
#include "app_ama_handle.h"
#ifdef VOICE_DETECTOR_SENS_EN
#include "mcu_sensor_hub_app_ai.h"
#endif

#undef ALIGN
#define ALIGN(n) __attribute__((aligned(n)))

// #define ALEXA_DUMP_AUDIO_DATA
/// define thirdparty function type as KWS
#define THIRDPARTY_FUNC_TYPE        THIRDPARTY_FUNC_KWS
/// signal for start hotword detection
#define ALEXA_START_DETECT_SIGNAL   (0x01)
/// signal for mic data come
#define ALEXA_DATA_COME_SIGNAL      (0x02)
/// signal for hotword detected
#define ALEXA_HW_DETECTED_SIGNAL    (0x03)
/// signal for upstreaming
#define ALEXA_DATA_UPSTREAM_SIGNAL    (0x04)
#define ALEXA_HW_DETECT_CALL_START       (0x05)
#define ALEXA_HW_DETECT_CALL_STOP       (0x06)
#define ALEXA_THREAD_KILL_SELF  (0xaa)

/// Set detection threshold for all keywords (this function can be called any time after decoder initialization)
#define DETECTION_THRESHOLD     500
#define PRL_BINARY_MODEL_LEN    52000

#define SAMPLES_PER_FRAME (160)
#define CHANNEL_NUM_MAX (1)
#define WWE_BUF_SIZE    1024*28

#define CODEC_STREAM_ID     AUD_STREAM_ID_0

#define FRAME_LEN               (SAMPLES_PER_FRAME)
#define CAPTURE_CHANNEL_NUM     (1)
#define FRAME_SIZE_LEN          (FRAME_LEN*2)
#define CAPTURE_BUF_SIZE        (FRAME_SIZE_LEN * CAPTURE_CHANNEL_NUM * 2)

#define WWE_HANDLE_FRAME_SIZE_LEN   (FRAME_LEN*4*2)
#define AI_SENT_FRAME_SIZE_LEN      (FRAME_LEN*2*2)

#ifdef VOICE_DETECTOR_EN
#define ALEXA_WWE_DETECT_MAX_TIMES 300
#if defined(CHIP_BEST2300A)
#define ALEXA_DATA_BUF_SIZE 1024*33
#else
#define ALEXA_DATA_BUF_SIZE 1024*17
#endif
#else
#define ALEXA_DATA_BUF_SIZE FRAME_SIZE_LEN*16
#endif

typedef void (*AlexaCallback)(void*, char);

typedef struct{
    uint32_t buf_len;
    uint8_t *model_buf;
}ALEXA_MODE_FILE_RAM_RUN_BUF_STRUCT_T;

/// binary model buffer, allocated by application
/// this buffer can be read-only memory as PryonLite will not modify the contents
/// should be an array large enough to hold the largest wakeword model
// ALIGN(4) static const char *wakewordModelBuffer = prlBinaryModelData;
#ifndef VOICE_DETECTOR_SENS_EN
#if defined(AI_KWS_ENGINE_OVERLAY)
ALIGN(4) uint8_t *model_buf = NULL;
//static char *engineBuffer[CHANNEL_NUM_MAX] = {NULL}; // should be an array large enough to hold the largest engine
static char *engineBuffer[CHANNEL_NUM_MAX] = {NULL}; // should be an array large enough to hold the largest engine
#else
ALIGN(4) uint8_t model_buf[PRL_BINARY_MODEL_LEN] = {0, };
static char engineBuffer[CHANNEL_NUM_MAX][WWE_BUF_SIZE] = {0, }; // should be an array large enough to hold the largest engine
#endif
#endif

ALEXA_MODE_FILE_RAM_RUN_BUF_STRUCT_T model_ram_run_instant;

#ifdef VOICE_DETECTOR_EN
extern short vad_buf[];
extern unsigned int vad_buf_len;

static uint32_t alexa_wwe_detect_times = 0;
static bool far_field_capture_start = false;
static bool alexa_wwe_vad_need_set_sysfreq = false;
#endif

#ifndef VOICE_DETECTOR_SENS_EN
extern size_t prlBinaryModelLen;    //51396
extern const char prlBinaryModelData[];

// engine handle
static PryonLiteV2Handle sHandle[CHANNEL_NUM_MAX] = {0};
static PryonLiteV2Config engineConfig[CHANNEL_NUM_MAX] = {0};
static PryonLiteV2EventConfig engineEventConfig = {0};
static PryonLiteV2ConfigAttributes configAttributes = {0};
static PryonLiteWakewordConfig wakewordConfig[CHANNEL_NUM_MAX] = {PryonLiteWakewordConfig_Default};

// static char *engineBuffer[CHANNEL_NUM_MAX] = {NULL}; // should be an array large enough to hold the largest engine

static short restFrameBuf[CHANNEL_NUM_MAX][SAMPLES_PER_FRAME] = { 0 };
static int restFrameLen[CHANNEL_NUM_MAX] = { 0 };
static AlexaCallback AlexaWweDemoCB = NULL;
static char AlexaWweCh = 0;
static bool alexa_lib_initied = false;

// CQueue alexa_data_buf_queue;
// uint8_t alexa_data_buf[ALEXA_DATA_BUF_SIZE];
uint8_t frame_data_buf[WWE_HANDLE_FRAME_SIZE_LEN];
// uint8_t *alexa_data_buf = NULL;
// uint8_t *frame_data_buf = NULL;
#else
static bool sensor_hub_kws_trigger = false;
#endif  //#ifndef VOICE_DETECTOR_SENS_EN

#if 0
uint32_t alexa_read_offset_in_ai_voice_buffer = 0;
#endif

static AIV_WAKEUP_INFO_T wake_word;

static bool alexa_wwe_initated = false;
osMutexDef(alexa_wwe_queue_mutex);
osMutexId alexa_wwe_queue_mutex_id;

static osThreadId alexa_thread_tid = NULL;
static void alexa_thread(void const *argument);
osThreadDef(alexa_thread, osPriorityNormal, 1, 1024*2, "alexa_hot_word");

#if defined(AI_KWS_ENGINE_OVERLAY)
#include "heap_api.h"

static void _alexa_wwe_model_init_op(void);
static void alexa_wwe_call_start(void);
static void alexa_wwe_call_stop(void);

osMutexDef(alexa_wwe_lite_hotword_external_model_op_mutex);
static osMutexId alexa_wwe_lite_hotword_external_model_op_mutex_id = NULL;

static bool _alexa_wwe_hotword_allow_run = false;

void alexa_wwe_lite_hotword_external_model_resource_release(void);

static inline void alexa_wwe_lite_hotword_external_model_op_mutex_alloc(void)
{
    if (alexa_wwe_lite_hotword_external_model_op_mutex_id == NULL) {
        alexa_wwe_lite_hotword_external_model_op_mutex_id = osMutexCreate(osMutex(alexa_wwe_lite_hotword_external_model_op_mutex));
    }
}

static inline void alexa_wwe_lite_hotword_external_model_op_mutex_req(void)
{
    osMutexWait(alexa_wwe_lite_hotword_external_model_op_mutex_id, osWaitForever);
}

static inline void alexa_wwe_lite_hotword_external_model_op_mutex_rls(void)
{
    osMutexRelease(alexa_wwe_lite_hotword_external_model_op_mutex_id);
}

static void alexa_wwe_lite_hotword_external_model_allow_update(bool op)
{
    alexa_wwe_lite_hotword_external_model_op_mutex_req();
    _alexa_wwe_hotword_allow_run = op;
    alexa_wwe_lite_hotword_external_model_op_mutex_rls();
}

static void alexa_wwe_lite_hotword_external_model_share_mem_update_notify(uint8_t user, uint8_t exist_user)
{
    bool allow = false;
    if(user != exist_user){
        if(user == AI_SPEC_AMA){
            allow = true;
        }else{
            allow = false;
            alexa_wwe_lite_hotword_external_model_resource_release();
        }
        alexa_wwe_lite_hotword_external_model_allow_update(allow);
        TRACE(4,"%s %d %d %d ",__func__,user,exist_user,allow);
    }
}

void alexa_wwe_lite_hotword_external_model_resource_alloc(void)
{
    alexa_wwe_lite_hotword_external_model_op_mutex_req();

    TRACE(2,"%s %p",__func__,__builtin_return_address(0));

    if(app_ai_share_mem_pool_total_size() < (PRL_BINARY_MODEL_LEN )){
        ASSERT(0,"%s size = %d %d ",__func__,app_ai_share_mem_pool_free_size(),PRL_BINARY_MODEL_LEN);
    }

    if(_alexa_wwe_hotword_allow_run){
        alexa_wwe_lite_hotword_external_model_op_mutex_rls();
        return ;
    }

    uint8_t * buf = NULL;
    app_ai_share_mem_pool_monolize_force(AI_SPEC_INIT,&buf,0);

    bool fail = false;
    uint8_t force =0;
    if(model_buf == NULL){
//        model_buf = (uint8_t*)share_mem_pool_heap_total_malloc(PRL_BINARY_MODEL_LEN);
        force += (uint8_t)app_ai_share_mem_pool_monolize_force(AI_SPEC_AMA,&model_buf,PRL_BINARY_MODEL_LEN);

        if(model_buf == NULL){
            fail = true;
        }
    }

    if(engineBuffer[0] == NULL){
//        model_buf = (uint8_t*)share_mem_pool_heap_total_malloc(PRL_BINARY_MODEL_LEN);
        force += (uint8_t)app_ai_share_mem_pool_monolize_force(AI_SPEC_AMA,(uint8_t**)&engineBuffer[0],WWE_BUF_SIZE);

        if(engineBuffer[0] == NULL){
            fail = true;
        }else{
            memset(&engineBuffer[0][0],0,WWE_BUF_SIZE);
#if 0
            engineBuffer[0][0] = 0x55;
            engineBuffer[0][WWE_BUF_SIZE -1] = 0xaa;
            uint16_t i = 0;
            uint16_t remain = 0;
            remain = WWE_BUF_SIZE / 1024;
            for(i=0;i<remain;i++){
                for(uint16_t j=0;j<1024;j++){
                    engineBuffer[0][i*1024 +j] = 0x55;
                }
                DUMP8("%0x ",&engineBuffer[0][i*1024],1024);
            }
            TRACE(2,"val check:%02x %02x",engineBuffer[0][0],engineBuffer[0][WWE_BUF_SIZE -1]);
#endif
        }
    }
    if (fail || force)
    {
        TRACE(3,"%s fail =%d force ?= %d",__func__,fail,force);
        app_ai_share_mem_pool_info_print();
        ASSERT(!fail,"need to check %d %d",fail,force);
    }

    TRACE(3,"%s %p %p",__func__,model_buf,engineBuffer[0]);
    _alexa_wwe_hotword_allow_run = true;
    alexa_wwe_lite_hotword_external_model_op_mutex_rls();
}

void alexa_wwe_lite_hotword_external_model_resource_release(void)
{
    alexa_wwe_lite_hotword_external_model_op_mutex_req();

    TRACE(2,"%s %p",__func__,__builtin_return_address(0));
    if(model_buf){
//        share_mem_pool_heap_total_free(model_buf);
        model_buf = NULL;
    }

#if 0
    if(engineBuffer[0]){
        engineBuffer[0] = NULL;
    }
#endif    
//    share_mem_pool_heap_total_info_print();

    _alexa_wwe_hotword_allow_run = false;

    alexa_wwe_lite_hotword_external_model_op_mutex_rls();
    app_ai_share_mem_pool_info_print();
}
#endif
static void _alexa_wwe_wake_word_status_reset(void)
{
    memset((uint8_t*)(&wake_word),0,sizeof(AIV_WAKEUP_INFO_T));
}

static void _alexa_wwe_lite_model_reload(void)
{
#if defined(AI_KWS_ENGINE_OVERLAY)
    alexa_wwe_lite_hotword_external_model_resource_alloc();
    model_ram_run_instant.buf_len = PRL_BINARY_MODEL_LEN;
#else
    model_ram_run_instant.buf_len = ARRAY_SIZE(model_buf);
#endif
    model_ram_run_instant.model_buf = model_buf;
}
static void init_wake_word_model_buf(void)
{
    _alexa_wwe_wake_word_status_reset();
    _alexa_wwe_lite_model_reload();
#ifndef VOICE_DETECTOR_SENS_EN
    TRACE(3,"%s %p %d", __func__, model_ram_run_instant.model_buf, prlBinaryModelLen);
    ASSERT(model_ram_run_instant.buf_len >= prlBinaryModelLen, "Alexa model buffer exceeded!");

    memcpy(model_ram_run_instant.model_buf, prlBinaryModelData, prlBinaryModelLen);
#endif
}
#ifndef VOICE_DETECTOR_SENS_EN
static int alexa_wwe_lib_init(AlexaCallback cb, int ch);
static void loadWakewordModel(PryonLiteWakewordConfig* config)
{
    // In order to detect keywords, the decoder uses a model which defines the parameters,
    // neural network weights, classifiers, etc that are used at runtime to process the audio
    // and give detection results.

    // Each model is packaged in two formats:
    // 1. A .bin file that can be loaded from disk (via fopen, fread, etc)
    // 2. A .cpp file that can be hard-coded at compile time

    init_wake_word_model_buf();
    config->sizeofModel = prlBinaryModelLen; // example value, will be the size of the binary model byte array
    config->model = model_ram_run_instant.model_buf; // pointer to model in memory
}

// VAD event handler
static void vadEventHandler(PryonLiteV2Handle *handle, const PryonLiteVadEvent* vadEvent)
{
    TRACE(1,"VAD state %d\n", (int) vadEvent->vadState);
}

// Wakeword event handler
static void wakewordEventHandler(PryonLiteV2Handle *handle, const PryonLiteWakewordResult* wwEvent)
{
    // unsigned char i = 0;
    uint32_t index_count = 0;

    index_count = (uint32_t)(wwEvent->endSampleIndex - wwEvent->beginSampleIndex);
    wake_word.score = wwEvent->confidence;
    wake_word.sIdx = 1000;
    wake_word.eIdx = wake_word.sIdx + index_count;
    wake_word.triggerType = AIV_WAKEUP_TRIGGER_KEYWORD;
    wake_word.user = AIV_USER_AMA;

    TRACE(4, "%s begin %d end %d index_count %d",
          __func__,
          (uint32_t)wwEvent->beginSampleIndex,
          (uint32_t)wwEvent->endSampleIndex,
          index_count);
    TRACE(2,"%s size %d", __func__, sizeof(wwEvent->keyword));
    TRACE(1,"Detected wakeword '%s'", wwEvent->keyword);

    osSignalSet(alexa_thread_tid, ALEXA_HW_DETECTED_SIGNAL);

#if 0
    for (i = 0; i < AlexaWweCh; i++)
    {
        if (&sHandle[i] == handle)
        {
            if (AlexaWweDemoCB)
            {
                (*AlexaWweDemoCB)(&wake_word, i);
            }
            break;
        }
    }
#endif
}

static void handleEvent(PryonLiteV2Handle *handle, const PryonLiteV2Event* event)
{
    if (event->vadEvent != NULL)
    {
        vadEventHandler(handle, event->vadEvent);
    }
    if (event->wwEvent != NULL)
    {
        wakewordEventHandler(handle, event->wwEvent);
    }
}

static int alexa_wwe_lib_init(AlexaCallback cb, int ch)
{
    TRACE(1, "%s", __func__);
    int i = 0;

    if (ch == 0 || ch > CHANNEL_NUM_MAX)
    {
        // handle error
        TRACE(2,"!!!!!!!!!!%s %d", __func__, __LINE__);
        return -1;
    }
    AlexaWweCh = ch;
    AlexaWweDemoCB = cb;

    for (i = 0; i < AlexaWweCh; i++)
    {
        loadWakewordModel(&wakewordConfig[i]);

        wakewordConfig[i].detectThreshold = 500; // default threshold
        wakewordConfig[i].useVad = 0;  // enable voice activity detector
        wakewordConfig[i].apiVersion = PRYON_LITE_API_VERSION;  // set apiVersion

        engineConfig[i].ww = &wakewordConfig[i];

        engineEventConfig.enableVadEvent = false;
        engineEventConfig.enableWwEvent = true;

        PryonLiteStatus status = PryonLite_GetConfigAttributes(&engineConfig[i], &engineEventConfig, &configAttributes);
        TRACE(2,"%s requiredMem %d", __func__, configAttributes.requiredMem); //27840

        if (status.publicCode != PRYON_LITE_ERROR_OK)
        {
            // handle error
            TRACE(3,"!!!!!!!!!!%s %d status %d", __func__, __LINE__, status.publicCode);
            return -1;
        }

        if (configAttributes.requiredMem > WWE_BUF_SIZE)
        {
            // handle error
            TRACE(2,"!!!!!!!!!!%s %d", __func__, __LINE__);
            return -1;
        }

        status = PryonLite_Initialize(&engineConfig[i], &sHandle[i], handleEvent, &engineEventConfig, (char*)engineBuffer[i], configAttributes.requiredMem);
        if (status.publicCode != PRYON_LITE_ERROR_OK)
        {
            TRACE(3,"!!!!!!!!!!%s %d status %d", __func__, __LINE__, status.publicCode);
            return -1;
        }

        status = PryonLiteWakeword_SetDetectionThreshold(sHandle[i].ww, NULL, DETECTION_THRESHOLD);
        if (status.publicCode != PRYON_LITE_ERROR_OK)
        {
            TRACE(3,"!!!!!!!!!!%s %d status %d", __func__, __LINE__, status.publicCode);
            return -1;
        }

        // Call the set client property API to inform the engine of client state changes
        status = PryonLite_SetClientProperty(&sHandle[i], CLIENT_PROP_GROUP_COMMON, CLIENT_PROP_COMMON_AUDIO_PLAYBACK, 1);
        if (status.publicCode != PRYON_LITE_ERROR_OK)
        {
            TRACE(3,"!!!!!!!!!!%s %d status %d", __func__, __LINE__, status.publicCode);
            return -1;
        }
    }

    TRACE(2,"ok %s %d", __func__, __LINE__);
    alexa_lib_initied = true;
    return 0;
}

static int alexa_audio_sample_input(const short *samples, int sampleCount, int ch)
{
    PryonLiteStatus status;
    int offset = 0;
    int copyLen = 0;
    int i = 0;
    int chSampleCnt;

    //TRACE(1,"%s enter", __func__);

    if (ch > AlexaWweCh)
    {
        TRACE(2,"fail %s %d", __func__, __LINE__);
        return -1;
    }
#if defined(AI_KWS_ENGINE_OVERLAY)
    alexa_wwe_lite_hotword_external_model_op_mutex_req();
    if(_alexa_wwe_hotword_allow_run == false){
        alexa_wwe_lite_hotword_external_model_op_mutex_rls();
        return -1;
    }
#endif

    for (i = 0; i < ch; i++)
    {
        chSampleCnt = sampleCount;
        if (restFrameLen[i]) {
            copyLen = (chSampleCnt > (SAMPLES_PER_FRAME - restFrameLen[i])) ? (SAMPLES_PER_FRAME - restFrameLen[i]) : chSampleCnt;
            ASSERT(SAMPLES_PER_FRAME >= copyLen*sizeof(short), "%s buf exceed: %d|%d", __func__, SAMPLES_PER_FRAME, copyLen*sizeof(short));
            memcpy(&restFrameBuf[i][restFrameLen[i]], &samples[offset], copyLen*sizeof(short));
            restFrameLen[i] += copyLen;
            chSampleCnt -= copyLen;
            offset += copyLen;
        }

        if (restFrameLen[i] == SAMPLES_PER_FRAME) {
            restFrameLen[i] = 0;
            status = PryonLite_PushAudioSamples(&sHandle[i], restFrameBuf[i], SAMPLES_PER_FRAME);
            if (status.publicCode != PRYON_LITE_ERROR_OK)
            {
                // handle error
                TRACE(2,"fail %s %d", __func__, __LINE__);
#if defined(AI_KWS_ENGINE_OVERLAY)
                alexa_wwe_lite_hotword_external_model_op_mutex_rls();
#endif
                return -1;
            }
        }

        while (chSampleCnt >= SAMPLES_PER_FRAME) {
            status = PryonLite_PushAudioSamples(&sHandle[i], &samples[offset], SAMPLES_PER_FRAME);
            if (status.publicCode != PRYON_LITE_ERROR_OK)
            {
                // handle error
                TRACE(2,"fail %s %d", __func__, __LINE__);
#if defined(AI_KWS_ENGINE_OVERLAY)
                alexa_wwe_lite_hotword_external_model_op_mutex_rls();
#endif
                return -1;
            }
            chSampleCnt -= SAMPLES_PER_FRAME;
            offset += SAMPLES_PER_FRAME;
        }

        if (chSampleCnt) {
            ASSERT(SAMPLES_PER_FRAME >= chSampleCnt*sizeof(short), "%s buf exceed: %d|%d", __func__, SAMPLES_PER_FRAME, chSampleCnt*sizeof(short));
            memcpy(restFrameBuf[i], &samples[offset], chSampleCnt*sizeof(short));
            restFrameLen[i] += chSampleCnt;
            offset += chSampleCnt;
        }
    }

    //TRACE(1,"%s leave", __func__);

#if defined(AI_KWS_ENGINE_OVERLAY)
    alexa_wwe_lite_hotword_external_model_op_mutex_rls();
#endif
    return 0;
}
#else
static uint32_t _alexa_hotword_kws_history_pcm_data_size_update(uint32_t raw_len)
{
    TRACE(2,"%s size = %d",__func__,raw_len);
    wake_word.kwsHistoryPcmDataSize = raw_len;
    return wake_word.kwsHistoryPcmDataSize;
}

uint32_t _alexa_hotword_kws_get_kws_valid_raw_data_len(void)
{
    return wake_word.eIdx*sizeof(int16_t);
}

static uint32_t _alexa_hotword_kws_valid_raw_pcm_sample_fetch(uint8_t * buf,uint32_t len)
{
    uint32_t valid_sample = _alexa_hotword_kws_get_kws_valid_raw_data_len();
    uint32_t raw_sample = wake_word.kwsHistoryPcmDataSize;

    return (valid_sample>raw_sample)?(0):(raw_sample - valid_sample);
}

static void _alexa_hotword_kws_state_update(uint8_t* info,uint16_t len)
{
    uint8_t buffer_fetch[100];
    uint16_t valid_len = sizeof(buffer_fetch);
    valid_len = (valid_len > len)?(len):(valid_len);
    memcpy(buffer_fetch,info,valid_len);
    ALEXA_KWS_Result_T * kws_result = (ALEXA_KWS_Result_T*)buffer_fetch;
    wake_word.score         = kws_result->score;
    wake_word.sIdx          = kws_result->start_index;
    wake_word.eIdx          = kws_result->end_index;
    wake_word.triggerType   = AIV_WAKEUP_TRIGGER_KEYWORD;
    wake_word.user          = AIV_USER_AMA;
    TRACE(4,"%s info : %d %d %d",__func__,wake_word.score,wake_word.sIdx,wake_word.eIdx);
    osSignalSet(alexa_thread_tid, ALEXA_HW_DETECTED_SIGNAL);
}
#endif

#if 0
static uint32_t _get_read_offset_in_ai_voice_buf(void)
{
    return alexa_read_offset_in_ai_voice_buffer;
}

static void _set_read_offset_in_ai_voice_buf(uint32_t offset)
{
    uint32_t buf_len = app_ai_voice_get_ai_voice_buf_len();

    //TRACE(1, "read_offset_in_ai_voice_buf:%d->%d", alexa_read_offset_in_ai_voice_buffer, offset % buf_len);

    alexa_read_offset_in_ai_voice_buffer = offset % buf_len;
}

static int _on_stream_started(void)
{
#ifdef __THIRDPARTY
    /// inform specific AI that mic opened
    app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_KWS, THIRDPARTY_HW_DETECT_START, AI_SPEC_AMA);
#endif

    return 0;
}
#endif
POSSIBLY_UNUSED static void _on_stream_stoped(void)
{
#ifdef __THIRDPARTY
    app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_KWS, THIRDPARTY_HW_DETECT_STOP, AI_SPEC_AMA);
#endif
}

POSSIBLY_UNUSED static void alexa_wwe_thread_kill_signal(void)
{
    osSignalSet(alexa_thread_tid, ALEXA_THREAD_KILL_SELF);
//    svcRtxThreadJoin(alexa_thread_tid);
}
static int alexa_wwe_init(bool init, const void *)
{
    if (alexa_wwe_initated)
    {
        TRACE(1,"%s has inited", __func__);
        return -1;
    }

    if (NULL == alexa_thread_tid)
    {
        TRACE(1,"%s osThreadCreate", __func__);
        alexa_thread_tid = osThreadCreate(osThread(alexa_thread), NULL);
        ASSERT(alexa_thread_tid, "Failed to Create alexa_thread");
    }

#ifndef VOICE_DETECTOR_SENS_EN
#if defined(AI_KWS_ENGINE_OVERLAY)
    alexa_wwe_lite_hotword_external_model_op_mutex_alloc();
    app_ai_share_mem_pool_monolized_info_update_notify_callback_reg(AI_SPEC_AMA,(uint32_t)alexa_wwe_lite_hotword_external_model_share_mem_update_notify);
    alexa_wwe_lite_hotword_external_model_resource_alloc();
    model_ram_run_instant.buf_len = PRL_BINARY_MODEL_LEN;
#else
    model_ram_run_instant.buf_len = ARRAY_SIZE(model_buf);
    model_ram_run_instant.model_buf = model_buf;
    if (alexa_wwe_lib_init(NULL, 1))
    {
        ASSERT(0, "%s fail", __func__);
    }
#endif
#else
    SENSOR_HUB_AI_OPERATOR_T alexa_operator = 
    {
        {SENSOR_HUB_AI_USER_ALEXA,},
        {0,},
        {
            {_alexa_hotword_kws_history_pcm_data_size_update,},
            {_alexa_hotword_kws_valid_raw_pcm_sample_fetch,},
            {_alexa_hotword_kws_get_kws_valid_raw_data_len,},
            {_alexa_hotword_kws_state_update,},
            {(sensor_hub_ai_mcu_mic_data_come_handler_t)(app_ai_voice_interface_of_raw_input_handler_get()),},
        },
    };
    sensor_hub_ai_mcu_register_ai_user(SENSOR_HUB_AI_USER_ALEXA,alexa_operator);
    app_sensor_hub_ai_mcu_activate_ai_user(SENSOR_HUB_AI_USER_ALEXA,1);
#endif

    app_ai_set_use_thirdparty(true, AI_SPEC_AMA);
	app_ai_enable_wakeword(true, AI_SPEC_AMA);
    alexa_wwe_initated = true;
    return 0;
}

/*
    copy model register parts of 'alexa_wwe_init'
*/
POSSIBLY_UNUSED static void _alexa_wwe_model_init_op(void)
{
#ifndef VOICE_DETECTOR_SENS_EN
#if defined(AI_KWS_ENGINE_OVERLAY)
    TRACE(3,"%s line %d %p",__func__,__LINE__,__builtin_return_address(0));
    alexa_wwe_lite_hotword_external_model_op_mutex_req();
    if((_alexa_wwe_hotword_allow_run == true) && 
        (alexa_lib_initied == true))
    {
        alexa_wwe_lite_hotword_external_model_op_mutex_rls();
        return ;
    }

    if (NULL == alexa_thread_tid)
    {
        TRACE(1,"%s osThreadCreate", __func__);
        alexa_thread_tid = osThreadCreate(osThread(alexa_thread), NULL);
        ASSERT(alexa_thread_tid, "Failed to Create alexa_thread");
    }

    alexa_wwe_lite_hotword_external_model_resource_alloc();
    model_ram_run_instant.buf_len = PRL_BINARY_MODEL_LEN;

#else
    model_ram_run_instant.buf_len = ARRAY_SIZE(model_buf);
#endif
    model_ram_run_instant.model_buf = model_buf;

    if(alexa_lib_initied == false){
        if (PryonLite_IsInitialized(&sHandle[0]))
        {
            TRACE(1,"%s IsInitialized", __func__);
            PryonLite_Destroy(&sHandle[0]);
        }

        if (alexa_wwe_lib_init(NULL, 1))
        {
            ASSERT(0, "%s fail", __func__);
        }
    }
    alexa_wwe_initated = true;

#else
    SENSOR_HUB_AI_OPERATOR_T alexa_operator = 
    {
        {SENSOR_HUB_AI_USER_ALEXA,},
        {0,},
        {
            {_alexa_hotword_kws_history_pcm_data_size_update,},
            {_alexa_hotword_kws_valid_raw_pcm_sample_fetch,},
            {_alexa_hotword_kws_get_kws_valid_raw_data_len,},
            {_alexa_hotword_kws_state_update,},
            {(sensor_hub_ai_mcu_mic_data_come_handler_t)(app_ai_voice_interface_of_raw_input_handler_get()),},
        },
    };
    sensor_hub_ai_mcu_register_ai_user(SENSOR_HUB_AI_USER_ALEXA,alexa_operator);
    app_sensor_hub_ai_mcu_activate_ai_user(SENSOR_HUB_AI_USER_ALEXA,1);
#endif
#if defined(AI_KWS_ENGINE_OVERLAY)
    alexa_wwe_lite_hotword_external_model_op_mutex_rls();
#endif
}
static int alexa_wwe_deinit(bool init, const void *)
{
    if (!alexa_wwe_initated)
    {
        TRACE(1,"%s don't inited", __func__);
        return -1;
    }

    if (alexa_thread_tid)
    {
        osThreadTerminate(alexa_thread_tid);
        ///alexa_wwe_thread_kill_signal();
        alexa_thread_tid = NULL;
    }

#ifndef VOICE_DETECTOR_SENS_EN
    if (PryonLite_IsInitialized(&sHandle[0]))
    {
        TRACE(1,"%s IsInitialized", __func__);
        PryonLite_Destroy(&sHandle[0]);
    }
#endif

    alexa_lib_initied = false;
    alexa_wwe_initated = false;
    return 0;
}

POSSIBLY_UNUSED static void alexa_wwe_call_start(void)
{
    TRACE(2,"%s %d %p",__func__,__LINE__,__builtin_return_address(0));
#if defined(AI_KWS_ENGINE_OVERLAY)
//    alexa_wwe_lite_hotword_external_model_resource_release();
    alexa_wwe_lite_hotword_external_model_op_mutex_req();
//    uint32_t lock = int_lock();
//    if(_alexa_wwe_hotword_allow_run == true)
#endif
    {
        alexa_wwe_deinit(false,NULL);
    }
#if defined(AI_KWS_ENGINE_OVERLAY)
//    int_unlock(lock);
    alexa_wwe_lite_hotword_external_model_resource_release();
    alexa_wwe_lite_hotword_external_model_op_mutex_rls();
#endif
//    osSignalSet(alexa_thread_tid, ALEXA_HW_DETECT_CALL_START);
    TRACE(2,"%s %d ",__func__,__LINE__);
}

POSSIBLY_UNUSED static void alexa_wwe_call_stop(void)
{
    TRACE(2,"%s %p",__func__,__builtin_return_address(0));

#if defined(AI_KWS_ENGINE_OVERLAY)
//    uint32_t lock = int_lock();
        if (PryonLite_IsInitialized(&sHandle[0]))
        {
            TRACE(1,"%s IsInitialized", __func__);
//            PryonLite_Destroy(&sHandle[0]);
        }

//    _alexa_wwe_model_init_op();
    alexa_wwe_init(true,NULL);
//    int_unlock(lock);
#endif

//    osSignalSet(alexa_thread_tid, ALEXA_HW_DETECT_CALL_STOP);
//    alexa_wwe_lite_hotword_external_model_op_mutex_req();
//    alexa_wwe_init(false,NULL);
//    alexa_wwe_lite_hotword_external_model_op_mutex_rls();
}

POSSIBLY_UNUSED static void alexa_wwe_disconnect(void)
{
        TRACE(2,"%s %d %p",__func__,__LINE__,__builtin_return_address(0));
#if defined(AI_KWS_ENGINE_OVERLAY)
    //    alexa_wwe_lite_hotword_external_model_resource_release();
        alexa_wwe_lite_hotword_external_model_op_mutex_req();
    //    uint32_t lock = int_lock();
#endif
        {
            alexa_wwe_deinit(false,NULL);
        }
#if defined(AI_KWS_ENGINE_OVERLAY)
    //    int_unlock(lock);
        alexa_wwe_lite_hotword_external_model_resource_release();
        alexa_wwe_lite_hotword_external_model_op_mutex_rls();
#endif
    //    osSignalSet(alexa_thread_tid, ALEXA_HW_DETECT_CALL_START);
        TRACE(2,"%s %d ",__func__,__LINE__);
}

static int alexa_wwe_start_detect(uint32_t param_ptr, uint32_t param_len)
{
    TRACE(1, "%s", __func__);
    /// inform the alexa thread to start detect
    alexa_wwe_init(true,NULL);
#if defined(AI_KWS_ENGINE_OVERLAY)
    // app_ai_share_mem_pool_init();
    // app_ai_share_mem_pool_monilized_info_init();
    _alexa_wwe_model_init_op();
#endif
    app_ai_voice_update_work_mode(AIV_USER_AMA, AIV_WORK_MODE_HW_DETECTING);
    osSignalSet(alexa_thread_tid, ALEXA_START_DETECT_SIGNAL);

    return 0;
}

static int alexa_wwe_data_handle(uint32_t param_ptr, uint32_t param_len)
{
    /// inform the alexa thread to handle data
    osSignalSet(alexa_thread_tid, ALEXA_DATA_COME_SIGNAL);

    return 0;
}

static int alexa_wwe_stop_detect(uint32_t param_ptr, uint32_t param_len)
{
    TRACE(1, "%s", __func__);
    return 0;
}

static uint32_t alexa_wwe_get_sysfreq(void)
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

static void alexa_thread(void const *argument)
{
    TRACE(0, "alexa_thread start running.");
    /// register AI voice related handlers
    osEvent evt;
    POSSIBLY_UNUSED uint32_t stime,etime;
    POSSIBLY_UNUSED uint32_t hot_word_start_time_in_ms = hal_sys_timer_get();
    POSSIBLY_UNUSED uint32_t total_hot_word_time_in_ms = 0;
    POSSIBLY_UNUSED uint32_t hot_word_test_times = 0;
    POSSIBLY_UNUSED uint32_t opus_start_time_in_ms = hal_sys_timer_get();
    POSSIBLY_UNUSED uint32_t total_opus_time_in_ms = 0;
    POSSIBLY_UNUSED uint32_t opus_test_times = 0;
    POSSIBLY_UNUSED uint32_t cpu_freq = alexa_wwe_get_sysfreq();
    POSSIBLY_UNUSED uint32_t cpu_freq_last = alexa_wwe_get_sysfreq();

    POSSIBLY_UNUSED uint32_t length_of_cqueue = 0;
    uint32_t libRequiredLen = 0;
    uint32_t queuedDataLen = 0;
    uint32_t readOffset = 0;

    while(1)
    {
        /// wait any signal
        evt = osSignalWait(0x00, osWaitForever);
        if(osEventSignal == evt.status)
        {
            if (ALEXA_DATA_COME_SIGNAL != evt.value.signals)
            {
                TRACE(3,"[%s] status = %x, signals = %d", __func__, evt.status, evt.value.signals);
            }
            switch (evt.value.signals)
            {
            case ALEXA_START_DETECT_SIGNAL:
                TRACE(0, "Start alexa detection.");
                /// init wake word model buffer to restart local hotword detection
                init_wake_word_model_buf();
                /// update read offset in AI voice data cache buffer
                _set_read_offset_in_ai_voice_buf(app_ai_voice_get_buf_write_offset());
                /// update AI work mode
                //app_ai_voice_update_work_mode(AIV_USER_AMA, AIV_WORK_MODE_HW_DETECTING);
                break;
            case ALEXA_DATA_COME_SIGNAL:
                length_of_cqueue = app_ai_voice_get_cached_len_according_to_read_offset(_get_read_offset_in_ai_voice_buf());
                if (length_of_cqueue >= WWE_HANDLE_FRAME_SIZE_LEN)
                {
#ifndef VOICE_DETECTOR_SENS_EN
                    stime = hal_sys_timer_get();
                    /// get data from AI pcm data queue
                    if (app_ai_voice_get_pcm_data(_get_read_offset_in_ai_voice_buf(), frame_data_buf, WWE_HANDLE_FRAME_SIZE_LEN))
                    {
                        /// update read offset
                        _set_read_offset_in_ai_voice_buf(_get_read_offset_in_ai_voice_buf() + WWE_HANDLE_FRAME_SIZE_LEN);
                        /// put audio samples into alexa lib
                        alexa_audio_sample_input((short *)frame_data_buf, WWE_HANDLE_FRAME_SIZE_LEN / 2, 1);
                        etime = hal_sys_timer_get();
                        total_hot_word_time_in_ms += TICKS_TO_MS(etime - stime);
                        hot_word_test_times++;
                        if (hot_word_test_times % 100 == 0)
                        {
                            cpu_freq = alexa_wwe_get_sysfreq();
                            TRACE(1, "ALEXA HOT WORD COST MIPS %d", cpu_freq * total_hot_word_time_in_ms / TICKS_TO_MS(etime - hot_word_start_time_in_ms));
                            TRACE(3, "hot word time %d test_times %d total time %d", total_hot_word_time_in_ms, hot_word_test_times, TICKS_TO_MS(etime - hot_word_start_time_in_ms));
                            TRACE(2, "cpu_freq_last %d cpu_freq %d", cpu_freq_last, cpu_freq);
                            if (cpu_freq_last != cpu_freq)
                            {
                                cpu_freq_last = cpu_freq;
                                hot_word_start_time_in_ms = hal_sys_timer_get();
                                total_hot_word_time_in_ms = 0;
                                hot_word_test_times = 0;
                            }
                        }
                    }
                              
#else
                    if (wake_word.score == 0)
                    {
                        break;
                    }
                    libRequiredLen = 2 * wake_word.eIdx;
                    queuedDataLen = app_ai_voice_get_pcm_queue_len();
                    if (queuedDataLen >= libRequiredLen){
                        osSignalSet(alexa_thread_tid, ALEXA_DATA_UPSTREAM_SIGNAL);
                    }
#endif
                }
                break;
            case ALEXA_HW_DETECTED_SIGNAL:
#ifndef VOICE_DETECTOR_SENS_EN
            // fall through
#else
            sensor_hub_kws_trigger = true;
            _set_read_offset_in_ai_voice_buf(app_ai_voice_get_buf_write_offset());
            break;
#endif
            case ALEXA_DATA_UPSTREAM_SIGNAL:
                libRequiredLen = 2 * wake_word.eIdx;
                queuedDataLen = app_ai_voice_get_pcm_queue_len();
                readOffset = _get_read_offset_in_ai_voice_buf();
                // ASSERT(queuedDataLen > libRequiredLen, "reuqired data exceed cached data! %d %d ",queuedDataLen,libRequiredLen);
                if (queuedDataLen < libRequiredLen)
                {
                    TRACE(2, "reuqired data exceed cached data! %d %d", queuedDataLen, libRequiredLen);
                    break;
                }
                if (wake_word.score == 0)
                {
                    TRACE(1, "score not matched! %d ", wake_word.score);
                    break;
                }
                TRACE(2,"%s score:%d", __func__, wake_word.score);
                TRACE(3,"readOffset:%d queued len:%d, wanted len:%d", readOffset, queuedDataLen, libRequiredLen);
                /// get the alexa lib required read offset
#ifndef VOICE_DETECTOR_SENS_EN
                readOffset = app_ai_voice_pcm_queue_read_dec(readOffset, libRequiredLen);
#else
                if(sensor_hub_kws_trigger == false){
                    TRACE(1,"sensor_hub_kws_trigger = %d",sensor_hub_kws_trigger);
                    break;
                }
                sensor_hub_kws_trigger = false;
#endif  //#ifndef VOICE_DETECTOR_SENS_EN
                if (!app_thirdparty_callback_handle(AI_SPEC_AMA, THIRDPARTY_WAKE_UP_CALLBACK, (void *)&wake_word, readOffset))
                {
                    /// elapse analysis
                    cpu_freq_last = alexa_wwe_get_sysfreq();
                    opus_start_time_in_ms = hal_sys_timer_get();
                    total_opus_time_in_ms = 0;
                    opus_test_times = 0;
                }
                else
                {
                    TRACE(0, "Upstream not allowed!!!");
                    init_wake_word_model_buf();
                    // ASSERT(0, "%s fix me!", __func__);
                }
                break;
                case ALEXA_HW_DETECT_CALL_STOP:
                {
                    _alexa_wwe_model_init_op();
                }
                break;
                case ALEXA_HW_DETECT_CALL_START:
                {
#if defined(AI_KWS_ENGINE_OVERLAY)
                    alexa_wwe_lite_hotword_external_model_resource_release();
#endif
                }
                break;
                case ALEXA_THREAD_KILL_SELF:
                {
                    osThreadTerminate(osThreadGetId());
                }
                break;
            default:
                break;
            }
        }
        else
        {
            TRACE(2,"[%s] ERROR: evt.status = %d", __func__, evt.status);
            continue;
        }
    }
}

THIRDPARTY_HANDLER_TAB(ALEXA_WWE_LIB_NAME)
{
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_INIT,              alexa_wwe_init),            false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_HW_DETECT_START,   alexa_wwe_start_detect),    false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_CALL_START,        alexa_wwe_call_start),      false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_CALL_STOP,         alexa_wwe_call_stop),   false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_MIC_DATA_COME,     alexa_wwe_data_handle),     false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_HW_DETECT_STOP,    alexa_wwe_stop_detect),     false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_AI_DISCONNECT,            alexa_wwe_disconnect),          false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_DEINIT,            alexa_wwe_deinit),          false,  NULL},
};

THIRDPARTY_HANDLER_TAB_SIZE(ALEXA_WWE_LIB_NAME)
