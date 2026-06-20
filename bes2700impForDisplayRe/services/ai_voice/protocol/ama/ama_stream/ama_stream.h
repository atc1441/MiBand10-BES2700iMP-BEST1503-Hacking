

#ifdef __cplusplus
extern "C" {
#endif

typedef enum  {
    AMA_CONTROL_STREAM_ID = 0,
    AMA_VOICE_STREAM_ID = 1, 
} STREAM_ID_E;

#define AMA_VER 1
#define AMA_FLAG 0  //reserved -- set to 0

#define AMA_SERIAL_NUM  "2021060800000000000"

#define AMA_DEVICE_NAME "BES"

#define AMA_DEVICE_TYPE "A3BAP6Z889UXHC"

#define AMA_INFO_CHAR_MAX_LEN    (20)

#define AMA_LOCALE  "en-US"

#define AMA_CTRL_STREAM_HEADER__INIT           \
    {                                          \
        0, 0, AMA_CONTROL_STREAM_ID, AMA_VER, 0 \
    }

#define AMA_AUDIO_STREAM_HEADER__INIT           \
    {                                          \
        0, 0, AMA_VOICE_STREAM_ID, AMA_VER, 0 \
    }

#define DEVICE_FEATURE_BATTERY_LEVEL     (1<<6)
#define DEVICE_FEATURE_ANC               (1<<7)
#define DEVICE_FEATURE_PASSTHROUGH       (1<<8)
#define DEVICE_FEATURE_WALK_WORD         (1<<9)
#define DEVICE_FEATURE_PRIVACY_MODE      (1<<10)
#define DEVICE_FEATURE_EQUALIZER         (1<<11)

#define STATE_ANC_LEVEL                 (0x205)
#define STATE_ANC_ENABLE                (0x210)
#define STATE_PASSTHROUGH               (0x211)
#define STATE_FB_ANC_LEVEL              (0x209)
#define STATE_WAKE_WORD_ENABLE          (0x352)
#define STATE_EQUALIZER_BASS            (0x450)
#define STATE_EQUALIZER_MID             (0x451)
#define STATE_EQUALIZER_TREBLE          (0x452)

#define ama_features  (DEVICE_FEATURE_ANC|DEVICE_FEATURE_PASSTHROUGH|DEVICE_FEATURE_BATTERY_LEVEL)

typedef struct
{
    uint16_t use16BitLength : 1;
    uint16_t flags          : 6;
    uint16_t streamId       : 5;
    uint16_t version        : 4;
    uint16_t length;
} ama_stream_header_t ;

typedef enum  {
    USER_CANCEL,
    TIME_OUT, 
    UNKNOWN,
} ama_stop_reason_e;


const char *ama_streamID2str(STREAM_ID_E stream_id);
void ama_control_stream_handler(uint8_t* ptr, uint32_t len, uint8_t dest_id);

bool ama_stream_header_parse(uint8_t *buffer, ama_stream_header_t *header);

void ama_reset_connection (uint32_t timeout, uint8_t dest_id);
void ama_start_speech_request(uint8_t dest_id);
void ama_stop_speech_request (ama_stop_reason_e stop_reason, uint8_t dest_id);
void ama_endpoint_speech_request(uint32_t dialog_id, uint8_t dest_id);


uint32_t app_ama_audio_stream_handle(void *param1, uint32_t param2, uint8_t param3);

void ama_notify_device_configuration (uint8_t is_assistant_override, uint8_t dest_id);
void ama_control_send_transport_ver_exchange(uint8_t param3);

#ifdef __cplusplus
}
#endif
