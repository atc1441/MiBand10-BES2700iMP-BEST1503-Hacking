#ifndef __VOICE_OPUS_H__
#define __VOICE_OPUS_H__

#include "opus_config.h"
#include "opus_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define OPUS_CAPTURE_TWO_CHANNEL

typedef struct
{
    uint32_t    heapSize;
    uint8_t     channelCnt;
    uint8_t     complexity;
    uint8_t     packetLossPercentage;
    uint8_t     sizePerSample;
    uint16_t    appType;
    uint16_t    bandWidth;
    uint32_t    bitRate;
    uint32_t    sampleRate;
    uint32_t    signalType;
    uint32_t    periodPerFrame;
    uint8_t     isUseVbr            :1;
    uint8_t     isConstraintUseVbr  :1;
    uint8_t     isUseInBandFec      :1;
    uint8_t     isUseDtx            :1;
    uint8_t     reserve             :4;

} VOICE_OPUS_CONFIG_T;

#ifdef OPUS_CAPTURE_TWO_CHANNEL
#define VOICE_OPUS_HEAP_SIZE            (29*1024)
#else
#define VOICE_OPUS_HEAP_SIZE            (25*1024)
#endif
#define VOICE_OPUS_STACK_SIZE           (29*1024)

#ifdef OPUS_CAPTURE_TWO_CHANNEL
#define VOICE_OPUS_CHANNEL_COUNT        (2)
#else
#define VOICE_OPUS_CHANNEL_COUNT        (1)
#endif

#define VOICE_OPUS_COMPLEXITY           (0)
#define VOICE_OPUS_PACKET_LOSS_PERC     (0)
#define VOICE_SIZE_PER_SAMPLE           (2) // 16 bits, 1 channel
#define VOICE_OPUS_APP                  (OPUS_APPLICATION_AUDIO)
#define VOICE_OPUS_BANDWIDTH            (OPUS_BANDWIDTH_FULLBAND)

#ifdef DUAL_MIC_RECORDING
#ifdef RECORDING_USE_OPUS_LOW_BANDWIDTH
#define VOICE_OPUS_BITRATE              (32000)
#else
#define VOICE_OPUS_BITRATE              (64000)
#endif
#define VOICE_OPUS_SAMPLE_RATE          (48000)
#else /// DUAL_MIC_RECORDING
#ifdef AI_32KBPS_VOICE
#define VOICE_OPUS_BITRATE              (32000)
#else
#define VOICE_OPUS_BITRATE              (16000)
#endif
#define VOICE_OPUS_SAMPLE_RATE          (16000)
#endif

#define VOICE_SIGNAL_TYPE               (OPUS_SIGNAL_VOICE)
#define VOICE_FRAME_PERIOD              (OPUS_FRAMESIZE_20_MS)

#define VOICE_OPUS_USE_VBR              (0)
#define VOICE_OPUS_CONSTRAINT_USE_VBR   (0)
#define VOICE_OPUS_USE_INBANDFEC        (0)
#define VOICE_OPUS_USE_DTX              (0)
#define VOICE_OPUS_FRAME_PERIOD_IN_MS   (20)

#define OPUS_COMPARESS_RATION                   (VOICE_OPUS_SAMPLE_RATE * VOICE_SIZE_PER_SAMPLE * 8 / VOICE_OPUS_BITRATE)
#define VOICE_OPUS_ENCODED_DATA_SIZE_PER_FRAME  (VOICE_OPUS_BITRATE * VOICE_OPUS_FRAME_PERIOD_IN_MS / 1000 / 8)
#define VOICE_OPUS_PCM_DATA_SIZE_PER_FRAME      (VOICE_OPUS_ENCODED_DATA_SIZE_PER_FRAME * OPUS_COMPARESS_RATION)
#define VOB_VOICE_OPUS_SAMPLE_CNT_PER_FRAME     VOB_PCM_SIZE_TO_SAMPLE_CNT(VOICE_OPUS_PCM_DATA_SIZE_PER_FRAME)

int voice_opus_init(VOICE_OPUS_CONFIG_T* pConfig, uint8_t* heapPtr, uint8_t* stackPtr);
int voice_opus_deinit(void);
int32_t voice_opus_encode(uint8_t *bitstream, uint8_t *speech, uint32_t sampleCount, uint8_t isReset);
uint32_t voice_opus_decode(uint8_t *speech, uint32_t speechLen, uint8_t *bitstream, uint32_t sampleCount, uint8_t isReset);

#ifdef __cplusplus
}
#endif

#endif  // __VOICE_OPUS_H__
