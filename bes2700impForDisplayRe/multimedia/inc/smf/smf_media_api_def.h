#pragma once

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*config*/
typedef struct {
    uint32_t rate;
    uint16_t channel;
    uint16_t bits;
    bool is_monopoly;
}smf_audio_param_t;

typedef struct {
	union {
		struct {
            uint32_t bitrate;
			uint8_t aot;
			uint8_t layer;
			uint8_t package;
			uint8_t vbr;
		}aac;
		struct {
			uint8_t bitPool;
			uint8_t channelMode;
			uint8_t allocMethod;
			uint8_t numBlocks;
			uint8_t numSubBands;
		}sbc;
        struct {
            uint8_t rev;
        }silk;
        struct {
            uint32_t frameDMs;
            uint32_t application;
            uint32_t bitrate;
            uint32_t lsb_depth;
            uint32_t complexity;
            uint32_t have_head;
        }opus;
	};
}smf_audio_enc_param_t;

typedef enum {
    SMF_AUDIO_OUTPUT_DEFAULT    = 0,
    SMF_AUDIO_OUTPUT_SPK        = 1 << 0,
    SMF_AUDIO_OUTPUT_A2DP       = 1 << 1,
    SMF_AUDIO_OUTPUT_I2S        = 1 << 2,
}SMF_AUDIO_OUTPUT_TYPE;

typedef enum{
    SMF_AUDIO_PROCESS_TYPE_SW = 1 << 0,
    SMF_AUDIO_PROCESS_TYPE_HW = 1 << 1,
    SMF_AUDIO_PROCESS_TYPE_QTY,
}SMF_AUDIO_PROCESS_ALGO_TYPE;

typedef struct {
    smf_audio_param_t params;
    bool audio_process_algo_enable;
    SMF_AUDIO_PROCESS_ALGO_TYPE type;
}smf_audio_output_speaker_t;

typedef struct {
    smf_audio_param_t params;
    uint32_t kfifo;
    const char* codec;
    smf_audio_enc_param_t enc;
}smf_audio_output_a2dp_t;

typedef struct {
    smf_audio_param_t params;
    uint32_t id;
    bool master;
    bool audio_process_algo_enable;
    SMF_AUDIO_PROCESS_ALGO_TYPE type;
}smf_audio_output_i2s_t;

typedef struct {
    uint32_t outputType;
    smf_audio_output_speaker_t outputSpk;
    smf_audio_output_a2dp_t outputA2dp;
    smf_audio_output_i2s_t outputI2s;
}smf_audio_output_config_t;

typedef struct {
    uint32_t rate;
    uint16_t channel;
    uint16_t bits;
    uint16_t dms;
}smf_audio_mix_config_t;

typedef enum {
    SMF_AUDIO_INPUT_DEFAULT    = 0,
    SMF_AUDIO_INPUT_MIC        = 1 << 0,//"mic"
    SMF_AUDIO_INPUT_I2S        = 1 << 1,
    SMF_AUDIO_INPUT_TDM        = 1 << 2,//"tdm"
    SMF_AUDIO_INPUT_ECHO       = 1 << 3,
}SMF_AUDIO_INPUT_TYPE;

typedef enum {
    SMF_AUDIO_INPUT_ECHO_DEFAULT = 0,
    SMF_AUDIO_INPUT_ECHO_I2S,
}SMF_AUDIO_INPUT_ECHO_TYPE;

typedef struct {
    smf_audio_param_t params;
    uint32_t mic_channel_map;
}smf_audio_input_mic_t;

typedef struct {
    smf_audio_param_t params;
    uint32_t id;
    bool master;
}smf_audio_input_i2s_t;

typedef struct {
    smf_audio_param_t params;
    uint32_t id;
    bool master;
    uint32_t fs_cycle;
    uint32_t slot_cycle;
}smf_audio_input_tdm_t;

typedef struct {
    smf_audio_param_t params;
    SMF_AUDIO_INPUT_ECHO_TYPE type;
    union{
        uint32_t temp[4];
        struct{
            uint32_t id;
            bool master;
        }i2s;
    };
}smf_audio_input_echo_t;

typedef struct {
    uint32_t inputType;
    smf_audio_input_mic_t   inputMic;
    smf_audio_input_i2s_t   inputI2s;
    smf_audio_input_tdm_t   inputTdm;
    smf_audio_input_echo_t  inputEcho;
}smf_audio_input_config_t;

typedef enum {
    SMF_AUDIO_ALGO = 0,
    SMF_AUDIO_ALGO_MIC,
    SMF_AUDIO_ALGO_SPK,
    SMF_AUDIO_ALGO_I2S_IN,
    SMF_AUDIO_ALGO_I2S_OUT,
    SMF_AUDIO_ALGO_BTSCO_CAPTURE,
}SMF_AUDIO_ALGO_TYPE;

typedef struct {
    SMF_AUDIO_ALGO_TYPE type;
    void* data;
    uint32_t data_len;
}smf_audio_algo_set_param_t, smf_audio_algo_get_param_t;

/*player*/

typedef enum {
    SMF_AUDIO_PLAYER_STATUS_NULL = 0,
    SMF_AUDIO_PLAYER_STATUS_START,
    SMF_AUDIO_PLAYER_STATUS_PAUSE,
    SMF_AUDIO_PLAYER_STATUS_RESUME,
    SMF_AUDIO_PLAYER_STATUS_EOS,
    SMF_AUDIO_PLAYER_STATUS_STOP,
}SMF_AUDIO_PLAYER_STATUS;

typedef enum {
    SMF_AUDIO_PLAYER_NULL = 0,
    SMF_AUDIO_PLAYER_FILE,
    SMF_AUDIO_PLAYER_A2DP,
    SMF_AUDIO_PLAYER_STREAM,
    SMF_AUDIO_PLAYER_I2S,
}SMF_AUDIO_PLAYER_TYPE;

typedef struct {
    unsigned keys;
    unsigned vals;
}smf_audio_several_tones_t;

typedef struct {
    const char* codec;
    uint32_t kfifo;
}smf_audio_player_a2dpsink_t;

typedef void (*SmfAudioPlayerCallback) (uint64_t id, SMF_AUDIO_PLAYER_STATUS cmd_id);

typedef struct {
    const char* filename;
    uint32_t startpos;
    uint32_t rate;
    uint16_t channel;
    uint16_t bits;
    SmfAudioPlayerCallback callback;
    smf_audio_several_tones_t *tones;
    uint32_t volume;
}smf_audio_player_file_t;

typedef void (*SmfAudioStreamPlayerCallback)(void* data, uint32_t *size, uint32_t max_size, uint8_t *eos);

typedef struct {
    const char* codec;
    uint32_t rate;
    uint16_t channel;
    uint16_t bits;
    SmfAudioStreamPlayerCallback callback;
    uint32_t volume;
}smf_audio_player_stream_t;

typedef struct{
    uint32_t id;
    uint32_t rate;
    uint16_t channel;
    uint16_t bits;
    bool master;
    uint32_t volume;
}smf_audio_player_i2s_t;

typedef enum {
    SMF_AUDIO_DEVICE_INTCODEC_RX = 0,
    SMF_AUDIO_DEVICE_INTCODEC_TX,
	SMF_AUDIO_DEVICE_BTPCM_RX,
	SMF_AUDIO_DEVICE_BTPCM_TX,
    SMF_AUDIO_DEVICE_I2S0_RX,
    SMF_AUDIO_DEVICE_I2S0_TX,
    SMF_AUDIO_DEVICE_I2S1_RX,
    SMF_AUDIO_DEVICE_I2S1_TX,
	SMF_AUDIO_DEVICE_TDM_I2S0_RX,
	SMF_AUDIO_DEVICE_TDM_I2S0_TX,
	SMF_AUDIO_DEVICE_TDM_I2S1_RX,
	SMF_AUDIO_DEVICE_TDM_I2S1_TX,
	SMF_AUDIO_DEVICE_MAX,
}SMF_AUDIO_DEVICE_TYPE;

typedef enum{
    SMF_AUDIO_DEVICE_STATUS_STOP = 0,
    SMF_AUDIO_DEVICE_STATUS_START,
}SMF_AUDIO_DEVICE_STATUS;

typedef void (*SmfAudioDeviceCallback)(SMF_AUDIO_DEVICE_STATUS status, SMF_AUDIO_DEVICE_TYPE type);

/*recoder*/
typedef void (*SmfAudioRecordCallback) (uint64_t id, void* buffer, uint32_t size);

typedef enum {
    SMF_AUDIO_RECORDER_NULL = 0,
    SMF_AUDIO_RECORDER_FILE,
    SMF_AUDIO_RECORDER_FUNC,
    SMF_AUDIO_RECORDER_I2S,
}SMF_AUDIO_RECORDER_TYPE;

typedef struct {
    smf_audio_param_t params;
    const char* codec;
    smf_audio_enc_param_t enc;
    const char* filename;
}smf_audio_recorder_file_t;

typedef struct {
    smf_audio_param_t params;
    const char* codec;
    smf_audio_enc_param_t enc;
    SmfAudioRecordCallback callback;
    uint32_t fsize;
}smf_audio_recorder_func_t;

typedef struct {
    smf_audio_param_t params;
    uint32_t id;
    bool master;
}smf_audio_recorder_i2s_t;

/*hook*/
typedef void (*SmfMediaHookCallback)(uint32_t id, void* buffer, int size);

/*kws*/
typedef void (*SmfAudioKwsCallback)(uint32_t result);

typedef struct {
    uint32_t rate;
    uint16_t channel;
    uint16_t bits;
    uint16_t frame_size;
    SmfAudioKwsCallback callback;
}smf_audio_algo_kws_t;

/*wakeup*/
typedef void (*SmfAudioWakeupCallback)(void* data, uint32_t size);
typedef struct {
    uint32_t rate;
    uint16_t channel;
    uint16_t bits;
    uint32_t keys;
    void* data;
    uint32_t size;
}smf_audio_algo_wakeup_t;

/*usb audio*/
typedef struct{
    bool is_algo_bypass;
    bool is_echo_hw;
    uint32_t capture_algo_rate;
    uint16_t capture_algo_channel;
    uint16_t capture_algo_bits;
    uint16_t capture_algo_procsize;
    uint32_t mic_channel_map;
    void *capture_algo_cfg_data;
    uint32_t capture_algo_cfg_data_size;
}smf_audio_usb_t;

typedef enum {
    SMF_AUDIO_USB_PROMPT_POWER_ON = 0,
    SMF_AUDIO_USB_PROMPT_POWER_OFF,
}SMF_AUDIO_USB_PROMPT_TYPE;

/*video*/

typedef enum {
    SMF_VIDEO_SOURCE_DEFAULT    = 0,
    SMF_VIDEO_SOURCE_BLE        = 1,
    SMF_VIDEO_SOURCE_LOCAL      = 2,
    SMF_VIDEO_SOURCE_USB        = 3,
}SMF_VIDEO_SOURCE_TYPE;

typedef enum {
    SMF_VIDEO_PLAYER_DEFAULT    = 0,
    SMF_VIDEO_PLAYER_MP4_H264   = 1,
    SMF_VIDEO_PLAYER_MP4_JPEG   = 2,
    SMF_VIDEO_PLAYER_H264       = 3,
    SMF_VIDEO_PLAYER_MJPEG      = 4,
    SMF_VIDEO_PLAYER_GIF        = 5,
}SMF_VIDEO_PLAYER_TYPE;

typedef enum {
    SMF_VIDEO_FORMAT_NULL       = 0,
    SMF_VIDEO_FORMAT_YUV420P    = 1,
    SMF_VIDEO_FORMAT_YUV420P16  = 2,
    SMF_VIDEO_FORMAT_YUV422P    = 3,
    SMF_VIDEO_FORMAT_YUYV       = 4,
    SMF_VIDEO_FORMAT_VYUY       = 5,
    SMF_VIDEO_FORMAT_UYVY       = 6,
    SMF_VIDEO_FORMAT_PAL8       = 7,
}SMF_VIDEO_IMAGE_FORMAT;

typedef struct {
    uint32_t buffer;
    uint32_t size;
    uint16_t width;
    uint16_t height;
    SMF_VIDEO_IMAGE_FORMAT format;
}smf_video_image_info_t;

/*mp4*/
typedef enum {
    SMF_MEDIA_MUX_STREAM_VIDEO = 1 << 0,
    SMF_MEDIA_MUX_STREAM_AUDIO = 1 << 1,
    SMF_MEDIA_MUX_STREAM_OPTION1 = 1 << 2,
    SMF_MEDIA_MUX_STREAM_OPTION2 = 1 << 3,
    SMF_MEDIA_MUX_STREAM_OPTION3 = 1 << 4,
    SMF_MEDIA_MUX_STREAM_OPTION4 = 1 << 5,
    SMF_MEDIA_MUX_STREAM_OPTION5 = 1 << 6,
    SMF_MEDIA_MUX_STREAM_OPTION6 = 1 << 7,
}SMF_MEDIA_MUX_STREAM_TYPE;

#define SMF_MEDIA_MUX_STREAM_MAX 8

typedef enum {
    SMF_MEDIA_MUX_VIDEO_STREAM_NULL = 0,
    SMF_MEDIA_MUX_VIDEO_STREAM_FUNC,
    SMF_MEDIA_MUX_VIDEO_STREAM_UVC,
}SMF_MEDIA_MUX_VIDEO_STREAM_TYPE;

typedef enum {
    SMF_MEDIA_MUX_AUDIO_STREAM_NULL = 0,
    SMF_MEDIA_MUX_AUDIO_STREAM_FUNC,
    SMF_MEDIA_MUX_AUDIO_STREAM_MIC,
}SMF_MEDIA_MUX_AUDIO_STREAM_TYPE;

typedef enum {
    SMF_MEDIA_MUX_OPTION_STREAM_NULL = 0,
    SMF_MEDIA_MUX_OPTION_STREAM_FUNC,
}SMF_MEDIA_MUX_OPTION_STREAM_TYPE;

typedef struct {
    smf_audio_input_mic_t mic;
}smf_media_mux_audio_stream_mic_t;

typedef struct {
    SMF_MEDIA_MUX_VIDEO_STREAM_TYPE stream_type;
    // union {
    //     smf_media_mux_video_stream_func func;
    //     smf_media_mux_video_stream_uac uac;
    // };
}smf_media_mux_video_stream_param_t;

typedef struct {
    SMF_MEDIA_MUX_AUDIO_STREAM_TYPE stream_type;
    union {
        // smf_media_mux_audio_stream_func func;
        smf_media_mux_audio_stream_mic_t mic;
    };
}smf_media_mux_audio_stream_param_t;

typedef struct {
    const char* filename;
    uint32_t timestamp0;
}smf_media_mux_sink_stream_param_t;

typedef struct {
    uint32_t type;
    smf_media_mux_video_stream_param_t video;
    smf_media_mux_audio_stream_param_t audio;
    // smf_option_stream_type option;
    smf_media_mux_sink_stream_param_t sink;
}smf_media_file_mux_param_t;

typedef struct {
    const char* codec_type;
    void* buffer;
    uint32_t size;
    uint32_t timestamp;
    uint32_t eos;
}smf_media_push_frame_t;

typedef struct {
    const char* filename;
    uint32_t startpos;
    uint32_t info_addr;
    uint8_t type;
}smf_video_player_file_param_t;

typedef struct {
    SMF_VIDEO_SOURCE_TYPE src_type;
    uint32_t info_addr;
    const char* v_type;
    uint16_t width;
    uint16_t height;
}smf_video_player_stream_param_t;

typedef struct {
    char artist[50];
    char title[50];
    char album[50];
    uint32_t duration;
    uint32_t total_pos;
}smf_audio_player_meta_info_t;

#ifdef __cplusplus
}
#endif
