#include "hearing_aid.h"
#include "audioflinger.h"
#include "heap_api.h"
#include "app_utils.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "audio_dump.h"
#include "cmsis_os.h"

// #define HEARING_AID_AUDIO_DUMP

#define HEARING_AID_BUFFER_SIZE (9*1024)

static uint8_t hearing_aid_buffer[HEARING_AID_BUFFER_SIZE] __attribute__((aligned(4)));

#define CACHE_2_UNCACHE(addr)      ((unsigned char *)((unsigned int)(addr) & ~(0x04000000)))

static heap_handle_t heap;

#define CODEC_STREAM_ID         (AUD_STREAM_ID_2)

#define SAMPLE_RATE             (48000)
#define SAMPLE_BITS             (24)
#define FRAME_MS                (3)

#if SAMPLE_BITS >= 16
typedef int32_t VOICE_PCM_T;
#else
typedef int16_t VOICE_PCM_T;
#endif

#define BUF_SIZE_PRE_CHANNEL    (sizeof(VOICE_PCM_T) * SAMPLE_RATE / 1000 * FRAME_MS)

#define CAPTURE_CHANNEL_MAP     (AUD_CHANNEL_MAP_CH0)
#define CAPTURE_CHANNEL_NUM     (AUD_CHANNEL_NUM_1)
#define CAPTURE_BUF_SIZE        (BUF_SIZE_PRE_CHANNEL * CAPTURE_CHANNEL_NUM * 2)

#define PLAYBACK_CHANNEL_NUM    (AUD_CHANNEL_NUM_1)
#define PLAYBACK_BUF_SIZE       (BUF_SIZE_PRE_CHANNEL * PLAYBACK_CHANNEL_NUM * 2)

static VOICE_PCM_T *cache_buffer = NULL;

static uint32_t codec_capture_callback(uint8_t *buf, uint32_t len)
{
    int POSSIBLY_UNUSED pcm_len = len / sizeof(VOICE_PCM_T) / CAPTURE_CHANNEL_NUM;
    VOICE_PCM_T POSSIBLY_UNUSED *pcm_buf[CAPTURE_CHANNEL_NUM];
    int interval_len = len * 2 / CAPTURE_CHANNEL_NUM;

    for (int i = 0; i < CAPTURE_CHANNEL_NUM; i++) {
        pcm_buf[i] = (VOICE_PCM_T *)(buf + i * interval_len);
    }

    // TRACE(2,"[%s] cnt = %d", __func__, codec_capture_cnt++);

    for (int i = 0; i < CAPTURE_CHANNEL_NUM; i++) {
        memcpy(cache_buffer + i * pcm_len, pcm_buf[i], pcm_len * sizeof(VOICE_PCM_T));
    }

#if defined(HEARING_AID_AUDIO_DUMP)
    audio_dump_add_channel_data(0, pcm_buf[0], pcm_len);
#endif

    return len;
}

static uint32_t codec_playback_callback(uint8_t *buf, uint32_t len)
{
    int POSSIBLY_UNUSED pcm_len = len / sizeof(VOICE_PCM_T) / PLAYBACK_CHANNEL_NUM;
    VOICE_PCM_T POSSIBLY_UNUSED *pcm_buf[PLAYBACK_CHANNEL_NUM];
    int interval_len = len * 2 / PLAYBACK_CHANNEL_NUM;

    for (int i = 0; i < PLAYBACK_CHANNEL_NUM; i++) {
        pcm_buf[i] = (VOICE_PCM_T *)(buf + i * interval_len);
    }

#if PLAYBACK_CHANNEL_NUM == CAPTURE_CHANNEL_NUM
    for (int i = 0; i < PLAYBACK_CHANNEL_NUM; i++) {
        memcpy(pcm_buf[i], cache_buffer + i * pcm_len, pcm_len * sizeof(VOICE_PCM_T));
    }
#else
error "impl your own algorithm"
#endif

#if defined(HEARING_AID_AUDIO_DUMP)
    audio_dump_add_channel_data(1, pcm_buf[0], pcm_len);
    audio_dump_run();
#endif

    // TRACE(2,"[%s] cnt = %d", __func__, codec_playback_cnt++);

    return len;
}

int hearing_aid_stream_start(bool on)
{
    uint32_t ret = 0;
    static bool isRun = false;

    if (isRun == on)
        return 0;

    if (on) {
        TRACE(1, "[%s] ON", __func__);

        af_set_priority(AF_USER_HEARING_AID, osPriorityHigh);

        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_104M);
        // TRACE(2, "[%s] sys freq calc : %d", __func__, hal_sys_timer_calc_cpu_freq(5, 0));

        // init heap, all buffer should be alloced from this heap
        heap = heap_register(hearing_aid_buffer, sizeof(hearing_aid_buffer));

        cache_buffer = heap_malloc(heap, BUF_SIZE_PRE_CHANNEL * CAPTURE_CHANNEL_NUM);

#if defined(HEARING_AID_AUDIO_DUMP)
        audio_dump_init(BUF_SIZE_PRE_CHANNEL / sizeof(VOICE_PCM_T), sizeof(BUF_SIZE_PRE_CHANNEL), 2);
#endif

        struct AF_STREAM_CONFIG_T stream_cfg;

        // capture stream
        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_map = CAPTURE_CHANNEL_MAP;
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)CAPTURE_CHANNEL_NUM;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;
        stream_cfg.vol = 12;
        stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC; // AUD_STREAM_USE_INT_CODEC2
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.handler = codec_capture_callback;
        stream_cfg.data_size = CAPTURE_BUF_SIZE;
        uint8_t *codec_capture_buf = heap_malloc(heap, stream_cfg.data_size);
        stream_cfg.data_ptr = CACHE_2_UNCACHE(codec_capture_buf);

        TRACE(3, "[%s] codec capture sample_rate: %d, data_size: %d", __func__, stream_cfg.sample_rate, stream_cfg.data_size);
        ret = af_stream_open(CODEC_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg);
        ASSERT(ret == 0, "codec capture failed: %d", ret);

        // playback stream
        memset(&stream_cfg, 0, sizeof(stream_cfg));
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)PLAYBACK_CHANNEL_NUM;
        stream_cfg.sample_rate = (enum AUD_SAMPRATE_T)SAMPLE_RATE;
        stream_cfg.bits = (enum AUD_BITS_T)SAMPLE_BITS;
        stream_cfg.vol = 12;
        stream_cfg.chan_sep_buf = true;
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC; // AUD_STREAM_USE_INT_CODEC3
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = codec_playback_callback;
        stream_cfg.data_size = PLAYBACK_BUF_SIZE;
        uint8_t *codec_playback_buf = heap_malloc(heap, stream_cfg.data_size);
        stream_cfg.data_ptr = CACHE_2_UNCACHE(codec_playback_buf);

        TRACE(3, "[%s] codec playback sample_rate: %d, data_size: %d", __func__, stream_cfg.sample_rate, stream_cfg.data_size);
        ret = af_stream_open(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK, &stream_cfg);
        ASSERT(ret == 0, "codec playback failed: %d", ret);

        // Start
        af_stream_start(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);
        af_stream_start(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
    } else {
        // Close stream
        af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
        af_stream_stop(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);

        af_stream_close(CODEC_STREAM_ID, AUD_STREAM_PLAYBACK);
        af_stream_close(CODEC_STREAM_ID, AUD_STREAM_CAPTURE);

        size_t total = 0, used = 0, max_used = 0;
        heap_memory_info(heap, &total, &used, &max_used);
        TRACE(3,"Hearing Aid MALLOC MEM: total - %d, used - %d, max_used - %d.", total, used, max_used);
        // ASSERT(used == 0, "[%s] used != 0", __func__);

        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);

        af_set_priority(AF_USER_HEARING_AID, osPriorityAboveNormal);

        TRACE(1, "[%s] OFF", __func__);
    }

    isRun = on;

    return 0;
}