#ifndef __APP_TENCENT_VOICEPATH_H__
#define __APP_TENCENT_VOICEPATH_H__
#if 0
#include "app_key.h"
#include "cqueue.h"
#include "hal_aud.h"

#include "app_tencent_smartvoice.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_OUTPUT_STREAMING			0
#define VOICEPATH_STREAMING				1

#define CPATURE_FUNC_INDEX_SMARTVOICE   0
#define CAPTURE_FUNC_INDEX_MIX_MIC      1

#define CAPTURE_FEATURE_BIT_OFFSET_SMARTVOICE    CPATURE_FUNC_INDEX_SMARTVOICE
#define CAPTURE_FEATURE_BIT_OFFSET_MIC_MIX       CAPTURE_FUNC_INDEX_MIX_MIC

#define CAPTURE_FEATURE_SMARTVOICE    (1 << CAPTURE_FEATURE_BIT_OFFSET_SMARTVOICE)
#define CAPTURE_FEATURE_MIC_MIX       (1 << CAPTURE_FEATURE_BIT_OFFSET_MIC_MIX)

#define CAPTURE_STREAM_MIC_DATA_HANDLER_CNT  2

#define HW_SIDE_TONE_ATTENUATION_COEF_IN_DISABLE_MODE	(0x1F*2)
#define HW_SIDE_TONE_MAX_ATTENUATION_COEF		60	// in unit of db
#define HW_SIDE_TONE_MIN_ATTENUATION_COEF		0	// in unit of db
#define HW_SIDE_TONE_FADE_IN_GAIN_STEP			10	// in unit of db
#define HW_SIDE_TONE_FADE_IN_TIME_STEP_IN_MS	30	// in unit of ms
#define HW_SIDE_TONE_USED_ATTENUATION_COEF		0	// in unit of db


typedef void (*capture_stream_init_func)(void);
typedef void (*capture_stream_deinit_func)(void);
typedef uint32_t (*capture_stream_mic_data_handler_func)(uint8_t* ptrData, uint32_t dataLen);

typedef struct
{
    capture_stream_init_func    initializer;
    capture_stream_deinit_func  deinitializer;
    capture_stream_mic_data_handler_func handler;    
} CAPTURE_STREAM_MIC_DATA_HANDLER_T;

typedef struct
{
    uint8_t                     registeredMicDataHandlerCnt;
    CAPTURE_STREAM_MIC_DATA_HANDLER_T    micDataHandler[CAPTURE_STREAM_MIC_DATA_HANDLER_CNT];
} CAPTURE_STREAM_ENV_T;

enum {
    APP_VOICEPATH_MESSAGE_ID_CUSTOMER_FUNC,
    APP_VOICEPATH_MESSAGE_ID_MAX
};
typedef void (*APP_VOICEPATH_MOD_HANDLE_T)(void);

void 	app_tencent_voicepath_custom_init(void);
void 	app_tencent_voicepath_init(void);

int 	app_tencent_voicepath_start_audio_stream(void);
int 	app_tencent_voicepath_stop_audio_stream(void);

void 	app_tencent_voicepath_enable_mic_mix_during_music(void);
void 	app_tencent_voicepath_disable_mic_mix_during_music(void);

void 	app_tencent_voicepath_key(APP_KEY_STATUS *status, void *param);

void 	app_tencent_voicepath_mailbox_put(uint32_t message_id, uint32_t param0, 
	uint32_t param1, uint32_t ptr);


void 		capture_stream_init_disctributor(void);
void 		capture_stream_deinit_disctributor(void);
uint32_t 	capture_stream_mic_data_disctributor(uint8_t* ptrBuf, uint32_t length);

void 		app_tencent_voicepath_init_voice_encoder(void);
void 		app_tencent_voicepath_deinit_voice_encoder(void);
int 		app_tencent_voicepath_mic_data_come(uint8_t* ptrBuf, uint32_t length);

void app_tencent_voicepath_set_pending_started_stream(uint8_t pendingStream, bool isEnabled);
bool app_tencent_voicepath_get_stream_pending_state(uint8_t pendingStream);
bool app_tencent_voicepath_get_stream_state(uint8_t stream);
void app_tencent_voicepath_set_stream_state(uint8_t stream, bool isEnabled);

void app_tencent_voicepath_enable_hw_sidetone(uint8_t micChannel, uint8_t attenuationGain);
void app_tencent_voicepath_disable_hw_sidetone(void);
void app_tencent_voicepath_set_hw_sidetone_gain(uint8_t attenuationGain);
uint8_t app_tencent_voicepath_get_hw_sidetone_gain(void);
bool app_tencent_voicepath_is_to_fade_in_hw_sidetone(void);

void app_tencent_voicepath_init_resampler(uint8_t channelCnt, uint32_t outputBufSize,
    uint32_t originalSampleRate, uint32_t targetSampleRate);
void app_tencent_voice_deinit_resampler(void);
void app_tencent_voicepath_resmple(uint8_t* ptrInput, uint32_t dataLen);
void app_tencent_voicepath_mic_data_encoding(void);
uint32_t app_tencent_voicepath_mic_data_process(uint8_t* ptrBuf, uint32_t length);
void app_pop_data_from_cqueue(CQueue* ptrQueue, uint8_t *buff, uint32_t len);     //此函数与app_tencent_smartvoice.cpp中app_tencent_pop_data_from_cqueue一致

uint32_t app_tencent_voice_catpure_interval_in_ms(void);

int app_tencent_voicepath_resampled_data_come(uint8_t* ptrBuf, uint32_t length);

void app_tencent_voicepath_bt_link_disconnected_handler(void); 
void app_tencent_voicepath_set_audio_output_sample_rate(enum AUD_SAMPRATE_T sampleRate);
enum AUD_SAMPRATE_T app_tencent_voicepath_audio_output_sample_rate(void);
void app_tencent_voicepath_set_audio_output_data_buf_size(uint32_t bufferSize);
uint32_t app_tencent_voicepath_audio_output_data_buf_size(void);

void app_tencent_voice_setup_resampler(bool isEnabled);
bool app_tencent_voice_is_resampler_enabled(void);

#ifdef __cplusplus
}
#endif

#endif
#endif

