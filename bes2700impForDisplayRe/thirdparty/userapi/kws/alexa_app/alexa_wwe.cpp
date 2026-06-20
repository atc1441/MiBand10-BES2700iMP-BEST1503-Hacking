#include "cmsis_os.h"
#include "cmsis.h"
#include "cqueue.h"
#include "hal_trace.h"
#include "bluetooth_bt_api.h"
#include "app_utils.h"
#include <string.h>
#include "hal_trace.h"
#include "audio_dump.h"
#include "hal_cmu.h"
#include "hal_aud.h"
#include "hal_timer.h"
#include "hal_sleep.h"
#include "audioflinger.h"
#include "app_thirdparty.h"
#include "cqueue.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pryon_lite_PRL1000.h"

#define THIRDPARTY_FUNC_TYPE THIRDPARTY_FUNC_KWS

typedef void (*AlexaCallback)(void*, char);

extern "C" int app_capture_audio_mempool_get_buff(uint8_t **buff, uint32_t size);
static int alexa_wwe_stream_start(bool on, const void *);
static int alexa_wwe_lib_init(AlexaCallback cb, int ch);

extern size_t prlBinaryModelLen; //233468
extern const char prlBinaryModelData[];

#define SAMPLES_PER_FRAME (160)
#define CHANNEL_NUM_MAX (1)

//#define true 1
static short restFrameBuf[CHANNEL_NUM_MAX][SAMPLES_PER_FRAME] = { 0 };
static int restFrameLen[CHANNEL_NUM_MAX] = { 0 };

// global flag to stop processing, set by application
//static int quit = 0;

#undef ALIGN
#define ALIGN(n) __attribute__((aligned(n)))

// engine handle
static PryonLiteV2Handle sHandle[CHANNEL_NUM_MAX] = {0};


static PryonLiteV2Config engineConfig[CHANNEL_NUM_MAX] = {0};
static PryonLiteV2EventConfig engineEventConfig = {0};
static PryonLiteV2ConfigAttributes configAttributes = {0};
static PryonLiteWakewordConfig wakewordConfig[CHANNEL_NUM_MAX] = {PryonLiteWakewordConfig_Default};

#define ALEXA_SIGNAL_ID 0xA5
static osThreadId alexa_thread_tid = NULL;
static void alexa_thread(void const *argument);
osThreadDef(alexa_thread, osPriorityNormal, 1, 1024*18, "alexa_hot_word");

#define WWE_BUF_SIZE    1024*23
//static char engineBuffer[CHANNEL_NUM_MAX][WWE_BUF_SIZE] = { 0 }; // should be an array large enough to hold the largest engine
static char *engineBuffer[CHANNEL_NUM_MAX] = {NULL}; // should be an array large enough to hold the largest engine

static bool wake_word_event_now = false;
static bool wake_word_event_stop = false;
static bool wake_word_event_comed = false;
static bool wake_word_provide_speech = false;
#define ALEXA_DATA_OUT_BUF_SIZE 1024*44
CQueue alexa_data_out_buf_queue;
//uint8_t alexa_data_out_buf[ALEXA_DATA_OUT_BUF_SIZE];
uint8_t *alexa_data_out_buf = NULL;
static AIV_WAKEUP_INFO_T wake_word;

static AlexaCallback AlexaDemoCB = NULL;
static char AlexaCh = 0;
static bool alexa_wwe_initated = false;
static bool alexa_wwe_ai_connected = false;
static bool alexa_wwe_ai_in_sco = false;

// binary model buffer, allocated by application
// this buffer can be read-only memory as PryonLite will not modify the contents
//ALIGN(4) static const char *wakewordModelBuffer = prlBinaryModelData; // should be an array large enough to hold the largest wakeword model
ALIGN(4) uint8_t *model_buf = NULL;

static void init_wake_word_model_buf(void)
{
    TRACE(2,"%s %d", __func__, prlBinaryModelLen);

#ifndef IS_MULTI_AI_ENABLED
    memcpy(model_buf, prlBinaryModelData, prlBinaryModelLen);
#endif
}

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
#ifdef IS_MULTI_AI_ENABLED
    config->model = prlBinaryModelData; // pointer to model in memory
#else
    config->model = model_buf; // pointer to model in memory
#endif
}

// VAD event handler
static void vadEventHandler(PryonLiteV2Handle *handle, const PryonLiteVadEvent* vadEvent)
{
    TRACE(1,"VAD state %d\n", (int) vadEvent->vadState);
}
// Wakeword event handler
static void wakewordEventHandler(PryonLiteV2Handle *handle, const PryonLiteWakewordResult* wwEvent)
{
    //unsigned char i = 0;
    uint32_t index_count = 0;

    index_count = (uint32_t)(wwEvent->endSampleIndex - wwEvent->beginSampleIndex);
    wake_word.score = wwEvent->confidence;
    wake_word.start_index = 1000;
    wake_word.end_index = wake_word.start_index + index_count;
    TRACE(4,"%s begin %d end %d index_count %d", __func__, \
                                        (uint32_t)wwEvent->beginSampleIndex, \
                                        (uint32_t)wwEvent->endSampleIndex, \
                                        index_count);

    TRACE(2,"%s size %d", __func__, sizeof(wwEvent->keyword));
    TRACE(1,"Detected wakeword '%s'\n", wwEvent->keyword);

    wake_word_event_now = true;
    osSignalSet(alexa_thread_tid, ALEXA_SIGNAL_ID);

#if 0
    for (i = 0; i < AlexaCh; i++)
    {
        if (&sHandle[i] == handle)
        {
            if (AlexaDemoCB)
            {
                (*AlexaDemoCB)(&wake_word, i);
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
    // Set detection threshold for all keywords (this function can be called any time after decoder initialization)
    int detectionThreshold = 500;
    int i = 0;

    if (ch == 0 || ch > CHANNEL_NUM_MAX)
    {
        // handle error
        TRACE(2,"!!!!!!!!!!%s %d", __func__, __LINE__);
        return -1;
    }
    AlexaCh = ch;
    AlexaDemoCB = cb;

    for (i = 0; i < AlexaCh; i++)
    {
        loadWakewordModel(&wakewordConfig[i]);

        wakewordConfig[i].detectThreshold = 500; // default threshold
        wakewordConfig[i].useVad = 0;  // enable voice activity detector

        engineConfig[i].ww = &wakewordConfig[i];

        engineEventConfig.enableVadEvent = false;
        engineEventConfig.enableWwEvent = true;

        PryonLiteError err = PryonLite_GetConfigAttributes(&engineConfig[i], &engineEventConfig, &configAttributes);
        TRACE(2,"%s requiredMem %d", __func__, configAttributes.requiredMem);   //23344

        if (err != PRYON_LITE_ERROR_OK)
        {
            // handle error
            TRACE(3,"!!!!!!!!!!%s %d err %d", __func__, __LINE__, err);
            return -1;
        }

        if (configAttributes.requiredMem > WWE_BUF_SIZE)
        {
            // handle error
            TRACE(2,"!!!!!!!!!!%s %d", __func__, __LINE__);
            return -1;
        }

        err = PryonLite_Initialize(&engineConfig[i], &sHandle[i], handleEvent, &engineEventConfig, (char*)engineBuffer[i], WWE_BUF_SIZE);
        if (err != PRYON_LITE_ERROR_OK)
        {
            TRACE(3,"!!!!!!!!!!%s %d err %d", __func__, __LINE__, err);
            return -1;
        }

        err = PryonLiteWakeword_SetDetectionThreshold(sHandle[i].ww, NULL, detectionThreshold);
        if (err != PRYON_LITE_ERROR_OK)
        {
            TRACE(3,"!!!!!!!!!!%s %d err %d", __func__, __LINE__, err);
            return -1;
        }
    }

    TRACE(2,"ok %s %d", __func__, __LINE__);
    return 0;
}

static bool wake_word_event_handle(void *param, char ch)
{
    uint32_t len_of_queue = 0;
    uint32_t len_of_delete = 0;
    AIV_WAKEUP_INFO_T *wake_word_ = (AIV_WAKEUP_INFO_T *)param;
    wake_word.triggerType = AIV_WAKEUP_TRIGGER_KEYWORD;

    len_of_queue = LengthOfCQueue(&alexa_data_out_buf_queue);
    TRACE(3,"%s score %d ch %d", __func__, wake_word_->score, ch);
    TRACE(2,"len %d end %d", len_of_queue, 2*wake_word_->end_index);
    if (alexa_wwe_ai_connected && !alexa_wwe_ai_in_sco)
    {
        if (len_of_queue > 2*wake_word_->end_index)
        {
            len_of_delete = len_of_queue - 2*wake_word_->end_index;
            TRACE(1,"len_of_delete %d", len_of_delete);
            DeCQueue(&alexa_data_out_buf_queue, NULL, len_of_delete);
        }

        if (!app_thirdparty_callback_handle(THIRDPARTY_WAKE_UP_CALLBACK, (void *)wake_word_, 0))
        {
            if (!app_thirdparty_callback_handle(THIRDPARTY_START_SPEECH_CALLBACK, NULL, 0))
            {
                return true;
            }
        }

        init_wake_word_model_buf();
    }

    return false;
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
    }
    if (NULL == alexa_thread_tid)
    {
        TRACE(0,"Failed to Create alexa_thread\n");
    }
    osSignalSet(alexa_thread_tid, 0x0);

    alexa_wwe_lib_init(NULL, 1);
    alexa_wwe_initated = true;
    wake_word_event_comed = false;
    wake_word_event_now = false;
    wake_word_event_stop = false;
    return 0;
}

static int alexa_wwe_deinit(bool init, const void *)
{
    if (!alexa_wwe_initated)
    {
        TRACE(1,"%s don't inited", __func__);
        return -1;
    }

    if (PryonLite_IsInitialized(&sHandle[0]))
    {
        TRACE(1,"%s IsInitialized", __func__);
        PryonLite_Destroy(&sHandle[0]);
    }

    if (alexa_thread_tid)
    {
        osThreadTerminate(alexa_thread_tid);
        alexa_thread_tid = NULL;
    }
    alexa_wwe_initated = false;
    return 0;
}

static int alexa_audio_sample_input(const short *samples, int sampleCount, int ch)
{
    PryonLiteError err;
    int offset = 0;
    int copyLen = 0;
    int i = 0;
    int chSampleCnt;

    //TRACE(1,"%s enter", __func__);

    if (ch > AlexaCh)
    {
        TRACE(2,"fail %s %d", __func__, __LINE__);
        return -1;
    }

    for (i = 0; i < ch; i++)
    {
        chSampleCnt = sampleCount;
        if (restFrameLen[i]) {
            copyLen = (chSampleCnt > (SAMPLES_PER_FRAME - restFrameLen[i])) ? (SAMPLES_PER_FRAME - restFrameLen[i]) : chSampleCnt;
            memcpy(&restFrameBuf[i][restFrameLen[i]], &samples[offset], copyLen*sizeof(short));
            restFrameLen[i] += copyLen;
            chSampleCnt -= copyLen;
            offset += copyLen;
        }

        if (restFrameLen[i] == SAMPLES_PER_FRAME) {
            restFrameLen[i] = 0;
            err = PryonLite_PushAudioSamples(&sHandle[i], restFrameBuf[i], SAMPLES_PER_FRAME);
            if (err != PRYON_LITE_ERROR_OK)
            {
                // handle error
                TRACE(2,"fail %s %d", __func__, __LINE__);
                return -1;
            }
        }

        while (chSampleCnt >= SAMPLES_PER_FRAME) {
            err = PryonLite_PushAudioSamples(&sHandle[i], &samples[offset], SAMPLES_PER_FRAME);
            if (err != PRYON_LITE_ERROR_OK)
            {
                // handle error
                TRACE(2,"fail %s %d", __func__, __LINE__);
                return -1;
            }
            chSampleCnt -= SAMPLES_PER_FRAME;
            offset += SAMPLES_PER_FRAME;
        }

        if (chSampleCnt) {
            memcpy(restFrameBuf[i], &samples[offset], chSampleCnt*sizeof(short));
            restFrameLen[i] += chSampleCnt;
            offset += chSampleCnt;
        }
    }

    //TRACE(1,"%s leave", __func__);

    return 0;
}

#define CODEC_STREAM_ID     AUD_STREAM_ID_0

#define FRAME_LEN               (SAMPLES_PER_FRAME)
#define CAPTURE_CHANNEL_NUM     (1)
#define FRAME_SIZE_LEN          (FRAME_LEN*2)
#define CAPTURE_BUF_SIZE        (FRAME_LEN * CAPTURE_CHANNEL_NUM * 2 * 2)

#define WWE_HANDLE_FRAME_SIZE_LEN   (FRAME_LEN*4*2)
#define AI_SENT_FRAME_SIZE_LEN      (FRAME_LEN*2*2)

static uint8_t codec_capture_buf[CAPTURE_BUF_SIZE] __attribute__((aligned(4)));

#define ALEXA_DATA_BUF_SIZE FRAME_SIZE_LEN*16
CQueue alexa_data_buf_queue;
//uint8_t alexa_data_buf[ALEXA_DATA_BUF_SIZE];
//uint8_t frame_data_buf[WWE_HANDLE_FRAME_SIZE_LEN];
uint8_t *alexa_data_buf = NULL;
uint8_t *frame_data_buf = NULL;

static uint32_t far_field_capture_callback(uint8_t *buf, uint32_t len)
{
    if (false == wake_word_event_comed)
    {
        EnCQueue_AI(&alexa_data_buf_queue, buf, len, NULL);
    }
    else
    {
        EnCQueue_AI(&alexa_data_out_buf_queue, buf, len, NULL);
    }
    osSignalSet(alexa_thread_tid, ALEXA_SIGNAL_ID);
    return len;
}

static int alexa_wwe_power_on_init(bool on, const void *)
{
    TRACE(1,"%s", __func__);

    app_capture_audio_mempool_get_buff(&alexa_data_buf, ALEXA_DATA_BUF_SIZE);
    app_capture_audio_mempool_get_buff(&frame_data_buf, WWE_HANDLE_FRAME_SIZE_LEN);
    app_capture_audio_mempool_get_buff(&alexa_data_out_buf, ALEXA_DATA_OUT_BUF_SIZE);
    app_capture_audio_mempool_get_buff((uint8_t **)engineBuffer, CHANNEL_NUM_MAX*WWE_BUF_SIZE);
#ifndef IS_MULTI_AI_ENABLED
    app_capture_audio_mempool_get_buff(&model_buf, 0);
#endif

    TRACE(6,"%s %p %p %p %p %p", __func__, 
                alexa_data_buf,
                frame_data_buf,
                alexa_data_out_buf,
                engineBuffer[0],
                model_buf);
    return 0;
}

static int alexa_wwe_stream_start(bool on, const void *)
{
    int ret = 0;
    static bool isRun =  false;
    struct AF_STREAM_CONFIG_T stream_cfg;

    TRACE(4,"%s work:%d op:%d init:%d", __func__, isRun, on, alexa_wwe_initated);
    if (!alexa_wwe_initated)
    {
        on = false;
        TRACE(1,"%s alexa_wwe don't initated", __func__);
        return 0;
    }

    if (!alexa_wwe_ai_connected)
    {
        TRACE(1,"%s alexa_wwe ai don't connect", __func__);
        return 0;
    }

    if (isRun == on)
    {
        TRACE(1,"%s isRun == on", __func__);
        return 0;
    }

    if (on)
    {
        app_sysfreq_req(APP_SYSFREQ_USER_AI_VOICE, APP_SYSFREQ_208M);
        af_set_priority(AF_USER_AI, osPriorityHigh);
        // TRACE(1,"sys freq calc: %d Hz", hal_sys_timer_calc_cpu_freq(5, 0));
        TRACE(2,"alexa_data_buf %p alexa_data_out_buf %p", alexa_data_buf, alexa_data_out_buf);

        InitCQueue(&alexa_data_buf_queue, ALEXA_DATA_BUF_SIZE, (CQItemType *)alexa_data_buf);
        InitCQueue(&alexa_data_out_buf_queue, ALEXA_DATA_OUT_BUF_SIZE, (CQItemType *)alexa_data_out_buf);

        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = AUD_CHANNEL_NUM_1;
        stream_cfg.data_size = CAPTURE_BUF_SIZE;
        stream_cfg.sample_rate = AUD_SAMPRATE_16000;
        stream_cfg.bits = AUD_BITS_16;
        stream_cfg.vol = 10;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg.io_path = AUD_INPUT_PATH_ASRMIC;
        stream_cfg.handler = far_field_capture_callback;
        stream_cfg.data_ptr = codec_capture_buf;

        TRACE(2,"codec capture sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);
        af_stream_open(CODEC_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
        ASSERT(ret == 0, "codec capture failed: %d", ret);

        af_stream_start(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);
    }
    else
    {
        // Close stream
        af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);
        af_stream_close(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);
        af_set_priority(AF_USER_AI, osPriorityAboveNormal);
        app_sysfreq_req(APP_SYSFREQ_USER_AI_VOICE, APP_SYSFREQ_32K);

        TRACE(0,"CAPTURE off");
    }

    isRun = on;
    return 0;
}

static int alexa_wwe_restart(void)
{
    TRACE(2,"%s ai connect %d",__func__, alexa_wwe_ai_connected);
    if (alexa_wwe_ai_connected && !alexa_wwe_ai_in_sco)
    {
        alexa_wwe_stream_start(false, NULL);
        init_wake_word_model_buf();
        alexa_wwe_stream_start(true, NULL);
    }
    return 0;
}

static void alexa_wwe_provide_speech_handle(void)
{
    uint32_t len_of_queue = 0;

    if (alexa_wwe_ai_connected && !alexa_wwe_ai_in_sco)
    {
        len_of_queue = LengthOfCQueue(&alexa_data_out_buf_queue);
        TRACE(2,"%s len %d", __func__, len_of_queue);
        if (len_of_queue)
        {
            DeCQueue(&alexa_data_out_buf_queue, NULL, len_of_queue);
        }

        app_thirdparty_callback_handle(THIRDPARTY_START_SPEECH_CALLBACK, NULL, 0);
    }
}

static int alexa_wwe_provide_speech(bool status, void *param)
{
    TRACE(2,"%s wake %d",__func__, wake_word_event_comed);
    if (!wake_word_event_comed)
    {
        wake_word_provide_speech = true;
        osSignalSet(alexa_thread_tid, ALEXA_SIGNAL_ID);
    }
    return 0;
}

static int alexa_wwe_stop_speech(bool status, void *param)
{
    TRACE(2,"%s wake %d",__func__, wake_word_event_comed);
    if (wake_word_event_comed)
    {
        wake_word_event_stop = true;
        osSignalSet(alexa_thread_tid, ALEXA_SIGNAL_ID);
    }
    return 0;
}

static int alexa_wwe_ai_connect(bool status, void *param)
{
    int ret = 0;

    TRACE(3,"%s ai connect %d sco %d", __func__, alexa_wwe_ai_connected, alexa_wwe_ai_in_sco);

    if (!alexa_wwe_ai_connected)
    {
        alexa_wwe_ai_connected = true;
        if (!alexa_wwe_ai_in_sco)
        {
            alexa_wwe_init(true, NULL);
            ret = alexa_wwe_stream_start(true, NULL);
        }
    }

    return ret;
}

static int alexa_wwe_ai_disconnect(bool status, void *param)
{
    int ret = 0;

    TRACE(3,"%s ai connect %d sco %d", __func__, alexa_wwe_ai_connected, alexa_wwe_ai_in_sco);

    if (alexa_wwe_ai_connected)
    {
        if (!alexa_wwe_ai_in_sco)
        {
            ret = alexa_wwe_stream_start(false, NULL);
            alexa_wwe_deinit(false, NULL);
            app_thirdparty_callback_handle(THIRDPARTY_STOP_SPEECH_CALLBACK, NULL, 0);
        }
        alexa_wwe_ai_connected = false;
    }

    return ret;
}

static int alexa_wwe_call_start(bool status, void *param)
{
    int ret = 0;
    TRACE(3,"%s ai connect %d sco %d", __func__, alexa_wwe_ai_connected, alexa_wwe_ai_in_sco);
    if (!alexa_wwe_ai_in_sco)
    {
        alexa_wwe_ai_in_sco = true;
        if (alexa_wwe_ai_connected)
        {
            ret = alexa_wwe_stream_start(false, NULL);
            alexa_wwe_deinit(false, NULL);
            app_thirdparty_callback_handle(THIRDPARTY_STOP_SPEECH_CALLBACK, NULL, 0);
        }
    }

    return ret;
}

static int alexa_wwe_call_stop(bool status, void *param)
{
    int ret = 0;
    TRACE(3,"%s ai connect %d sco %d", __func__, alexa_wwe_ai_connected, alexa_wwe_ai_in_sco);
    if (alexa_wwe_ai_in_sco)
    {
        alexa_wwe_ai_in_sco = false;
        if (alexa_wwe_ai_connected)
        {
            alexa_wwe_init(true, NULL);
            ret = alexa_wwe_stream_start(true, NULL);
        }
    }

    return ret;
}

static void alexa_thread(void const *argument)
{
    osEvent evt;
    uint32_t stime,etime;
    //uint8_t ai_data_handle_time = 0;

    while(1)
    {
        //wait any signal
        evt = osSignalWait(ALEXA_SIGNAL_ID, osWaitForever);
        //TRACE(3,"[%s] status = %x, signals = %d", __func__, evt.status, evt.value.signals);

        if(evt.status == osEventSignal)
        {
            if (!wake_word_event_comed)
            {
                if (wake_word_event_now)
                {
                    TRACE(1,"%s wake_word_event_now", __func__);
                    wake_word_event_now = false;
                    if (true == wake_word_event_handle(&wake_word, 1))
                    {
                        wake_word_event_comed = true;
                    }
                }
                else if (wake_word_provide_speech)
                {
                    TRACE(1,"%s wake_word_provide_speech", __func__);
                    wake_word_provide_speech = false;
                    alexa_wwe_provide_speech_handle();
                    wake_word_event_comed = true;
                }
                else if (LengthOfCQueue(&alexa_data_buf_queue) >= WWE_HANDLE_FRAME_SIZE_LEN)
                {
                    //stime=hal_sys_timer_get();
                    DeCQueue(&alexa_data_buf_queue, frame_data_buf, WWE_HANDLE_FRAME_SIZE_LEN);
                    EnCQueue_AI(&alexa_data_out_buf_queue, frame_data_buf, WWE_HANDLE_FRAME_SIZE_LEN, NULL);
                    alexa_audio_sample_input((short *)frame_data_buf, WWE_HANDLE_FRAME_SIZE_LEN/2, 1);
                    //etime=hal_sys_timer_get();
                    //TRACE(1,"ALEXA COST TIME %d", TICKS_TO_MS(etime-stime));
                    //TRACE(2,"%s ====>sys freq calc : %d\n", __func__, hal_sys_timer_calc_cpu_freq(50, 0));
                }
            }
            else
            {
                if (wake_word_event_stop)
                {
                    TRACE(1,"%s wake_word_event_stop", __func__);
                    wake_word_event_stop = false;
                    app_thirdparty_callback_handle(THIRDPARTY_STOP_SPEECH_CALLBACK, NULL, 0);
                    alexa_wwe_restart();
                    wake_word_event_comed = false;
                }
                else if (LengthOfCQueue(&alexa_data_out_buf_queue) >= AI_SENT_FRAME_SIZE_LEN)
                {
                    //for(ai_data_handle_time=0; ai_data_handle_time<2; ai_data_handle_time++)
                    {
                        TRACE(2,"%s len %d", __func__, LengthOfCQueue(&alexa_data_out_buf_queue));
                        stime=hal_sys_timer_get();
                        DeCQueue(&alexa_data_out_buf_queue, frame_data_buf, AI_SENT_FRAME_SIZE_LEN);
                        app_thirdparty_callback_handle(THIRDPARTY_DATA_COME_CALLBACK, (void *)frame_data_buf, AI_SENT_FRAME_SIZE_LEN);
                        etime=hal_sys_timer_get();
                        TRACE(1,"OPUS COST TIME %d", TICKS_TO_MS(etime-stime));
                        //osDelay(5);
                        //TRACE(2,"%s ====>sys freq calc : %d\n", __func__, hal_sys_timer_calc_cpu_freq(50, 0));
                    }
                }
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
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_INIT,              alexa_wwe_power_on_init),   false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_AI_PROVIDE_SPEECH, alexa_wwe_provide_speech),  false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_AI_STOP_SPEECH,    alexa_wwe_stop_speech),     false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_AI_CONNECT,        alexa_wwe_ai_connect),      false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_AI_DISCONNECT,     alexa_wwe_ai_disconnect),   false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_CALL_START,        alexa_wwe_call_start),      false,  NULL},
    {TP_EVENT(THIRDPARTY_ID_ALEXA_WWE, THIRDPARTY_CALL_STOP,         alexa_wwe_call_stop),       false,  NULL},
};

THIRDPARTY_HANDLER_TAB_SIZE(ALEXA_WWE_LIB_NAME)
