#ifndef __AUDIO_PROMPT_SBC_H__
#define __AUDIO_PROMPT_SBC_H__
#include "stdint.h"
#include "cqueue.h"
#include "bluetooth_bt_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIX_WITH_A2DP_STREAMING  0
#define MIX_WITH_SCO_STREAMING   1
#define MIX_WITH_BLE_AUDIO_MEDIA_STREAMING      2
#define MIX_WITH_BLE_AUDIO_FLEXIBLE_STREAMING   3

#define PENDING_TO_STOP_A2DP_STREAMING  0
#define PENDING_TO_STOP_SCO_STREAMING   1
#define PENDING_TO_STOP_BLE_AUDIO_STREAMING          2
#define PENDING_TO_STOP_BLE_AUDIO_SINGLE_STREAMING   3
#define PENDING_TO_STOP_STREAM_INVALID  (0xFF)

typedef struct
{
    uint8_t    channelCnt;
    uint8_t    channelMode;
    uint8_t    bitPool;
    uint8_t    sizePerSample;
    uint8_t    sampleRate;
    uint8_t    numBlocks;
    uint8_t    numSubBands;
    uint8_t    mSbcFlag;
    uint8_t    allocMethod;
} AUDIO_PROMPT_SBC_CONFIG_T;

#ifdef TWS_PROMPT_SYNC
typedef struct {
    uint16_t promptId;
    uint16_t promptPram;
    uint32_t sampleRate;
    uint32_t trigger_time;
    bt_bdaddr_t remote;
}APP_TWS_CMD_MIX_PROMPT_SYNC_T;

#endif

typedef struct
{
    uint8_t                 dev_id;
    uint16_t                voicePrompt;
    uint16_t                prompt_type;
} app_tws_voice_prompt_to_play_t;

#define IS_IGNORME_SBC_FRAME_HEADER             (1)
#define AUDIO_PROMPT_SBC_FRAME_HEADER_LEN       (4)

#define AUDIO_PROMPT_SBC_CHANNEL_COUNT          (1)
#define AUDIO_PROMPT_SBC_CHANNEL_MODE           (BTIF_SBC_CHNL_MODE_MONO)

#define AUDIO_PROMPT_SBC_BLOCK_SIZE             40
#define AUDIO_PROMPT_SBC_SIZE_PER_SAMPLE        (2)// 16 bits, 1 channel

#if 1    // by default it's 16000 sample rate
// Set this macro to 8000 if you want to use 8k sample rate ringtone
#define AUDIO_PROMPT_SBC_SAMPLE_RATE_VALUE      16000

#define AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE     (512)
#if (AUDIO_PROMPT_SBC_SAMPLE_RATE_VALUE == 8000)
#define AUDIO_PROMPT_TARGET_PCM_BUFFER_SIZE     (AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE*4)
#else
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON) || defined(A2DP_SCALABLE_ON) || defined(A2DP_LDAC_ON) 
#define AUDIO_PROMPT_TARGET_PCM_BUFFER_SIZE     (AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE*3)
#else
#define AUDIO_PROMPT_TARGET_PCM_BUFFER_SIZE     (AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE*2)
#endif
#endif
#define AUDIO_PROMPT_PCM_BUFFER_SIZE            (AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE*4)

#define AUDIO_PROMPT_SBC_FRAME_PERIOD_IN_MS     (24)

#define AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME        256
#else
#define AUDIO_PROMPT_SBC_SAMPLE_RATE_VALUE      48000  // could be changed to 32000 or 48000 if needed

#define AUDIO_PROMPT_SBC_FRAME_PERIOD_IN_MS     32

#define AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME        512
#define AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE     (1024)
#define AUDIO_PROMPT_TARGET_PCM_BUFFER_SIZE     (AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE*2)
#define AUDIO_PROMPT_PCM_BUFFER_SIZE            (AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE*4)
#endif

#define AUDIO_PROMPT_PCM_FILL_UNIT_SIZE                 (AUDIO_PROMPT_SBC_PCM_DATA_SIZE_PER_FRAME*2)

#define AUDIO_PROMPT_SBC_ENCODED_DATA_SIZE_PER_FRAME    (AUDIO_PROMPT_SBC_BLOCK_SIZE*4)

#define AUDIO_PROMPT_BUF_SIZE_FOR_RESAMPLER             800

typedef void (*mix_audio_stop_func_t)  (uint8_t,uint8_t,uint8_t);
uint32_t audio_prompt_sbc_get_frame_len(void);

void audio_prompt_init_handler(void);

bool audio_prompt_is_playing_ongoing(void);

void audio_prompt_buffer_config(uint8_t mixType,
                                uint8_t channel_cnt,
                                uint8_t bitNumPerSample,
                                uint8_t *tmpSourcePcmDataBuf,
                                uint8_t *tmpTargetPcmDataBuf,
                                uint8_t *pcmDataBuf,
                                uint32_t pcmBufLen,
                                uint8_t *bufForResampler,
                                uint32_t resampleBufLen);

bool audio_prompt_start_playing(uint16_t promptId, uint32_t targetSampleRate);

void audio_prompt_stop_playing(void);

void audio_prompt_processing_handler(uint8_t device_id, uint32_t acquiredPcmDataLen,
                                     uint8_t *pcmDataToMerge);

bool audio_prompt_check_on_stopping_stream(uint8_t pendingStopOp, uint8_t deviceId);

bool audio_prompt_check_on_stopping_ble_audio_stream(uint8_t pendingStopOp, uint8_t con_id,
                                                    uint32_t context_type, void *mix_audio_stop_func);

void audio_prompt_forcefully_stop(void);

bool audio_prompt_is_allow_update_volume(void);

uint16_t audio_prompt_get_prompt_id(void);

uint32_t audio_prompt_get_sample_rate(void);

#ifdef TWS_PROMPT_SYNC
void tws_playback_ticks_check_for_mix_prompt(uint8_t device_id);
void app_tws_cmd_sync_mix_prompt_req_handler(uint8_t *ptrParam, uint16_t paramLen);
#endif

void app_tws_cmd_prompt_play_req_handler(uint8_t *ptrParam, uint32_t paramLen);

bool app_tws_let_peer_device_play_audio_prompt(uint16_t promptId,
    uint16_t aud_param, uint8_t dev_id);

bool audio_prompt_clear_pending_stream(uint8_t op);

#ifdef __cplusplus
}
#endif

#endif	// __AUDIO_PROMPT_SBC_H__

