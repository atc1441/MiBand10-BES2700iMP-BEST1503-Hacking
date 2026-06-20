#ifndef __VOICE_COMPRESSION_H__
#define __VOICE_COMPRESSION_H__

#ifdef VOC_ENCODE_OPUS
#include "voice_opus.h"
#endif

#ifdef VOC_ENCODE_SBC
#include "voice_sbc.h"
#endif

#include "app_ai_if_config.h"

#define OPUS_ONESHOT_HANDLE_INTERVAL_IN_MS      (20)

#define VOC_CODEC_DEF(s)  _ENUM_DEF(VOC_CODEC_, s)
#define VOC_CODEC_LIST \
        VOC_CODEC_DEF(INVALID), \
        VOC_CODEC_DEF(ADPCM), \
        VOC_CODEC_DEF(OPUS), \
        VOC_CODEC_DEF(SBC), \
        VOC_CODEC_DEF(SCALABLE)

typedef enum
{
    VOC_CODEC_LIST,

    VOC_CODEC_CNT,
} VOC_CODEC_E;

typedef struct {
    uint8_t     algorithm;
    uint8_t     encodedFrameSize;
    uint16_t    pcmFrameSize;
    uint16_t    frameCountPerXfer;
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
    uint8_t     isUseVbr;
    uint8_t     isConstraintUseVbr;
    uint8_t     isUseInBandFec;
    uint8_t     isUseDtx;
} __attribute__((packed)) VOICE_OPUS_CONFIG_INFO_T;

typedef struct {
    uint8_t     algorithm;
    uint8_t     encodedFrameSize;
    uint16_t    pcmFrameSize;
    uint16_t    frameCountPerXfer;
    uint16_t    blockSize;
    uint8_t     channelCnt;
    uint8_t     channelMode;
    uint8_t     bitPool;
    uint8_t     sizePerSample;
    uint8_t     sampleRate;
    uint8_t     numBlocks;
    uint8_t     numSubBands;
    uint8_t     mSbcFlag;
    uint8_t     allocMethod;
} __attribute__((packed)) VOICE_SBC_CONFIG_INFO_T;

#ifdef VOC_ENCODE_SCALABLE
#define VALID_PCM_FRAME_SIZE    480  //!< The number of input audio sample for encoding according to scalable spec
#define DEFAULT_SYNC_WORD       0xA5 //!< The sync word in frame header according to the scalable spec
#define CHMODE_MONO             1    //!< channel mode used to initialize encoder
#define CHMODE_STEREO           3    //!< channel mode used to initialize encoder
#define SCALABLE_HEADER_SIZE    3    //!< 3 byte header according to the spec

typedef enum
{
    CHNL_MODE_STEREO_IDX = 0,
    CHNL_MODE_MONO_IDX = 1,

    CHNL_MODE_IDX_NUM,
} CHNL_MODE_IDX_E;

typedef enum
{
    BITRATE_MODE_0 = 0,        //!< mono output : 166byte, stereo output : 332byte
    BITRATE_MODE_1 = 1,        //!< mono output : 128byte, stereo output : 256byte
    BITRATE_MODE_2 = 2,        //!< mono output : 89byte, stereo output : 178byte

    BITRATE_MODE_NUM,
} BITRATE_MODE_E;

typedef struct
{
    uint8_t sync;
    uint8_t crc;
    uint8_t index : 6;
    uint8_t bitrate : 2;
} SCALABLE_FRAME_HEADER_T;
#endif

typedef enum
{
    COMPRESSION_STATE_OFF       = 0,
    COMPRESSION_STATE_ON        = 1,
} COMPRESSION_STATE_E;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef VOC_ENCODE_SCALABLE
/**
 * @brief Get the scalable encoded fram length
 * 
 * @param br            bitrate
 * @return int          frame length
 */
int voice_compression_get_scalable_frame_length(uint8_t br);
#endif

/**
 * @brief Get the compression state
 * 
 * @return uint8_t      The state of compression, @see COMPRESSION_STATE_E
 */
uint8_t voice_compression_get_state (void);

/**
 * @brief Get the compression codec
 * 
 * @return uint8_t      The codec of compression, @see COMPRESSION_CODEC_E
 */
uint8_t voice_compression_get_codec(void);

/**
 * @brief Get the compression user
 * 
 * @return uint8_t      The user of compression, @see COMPRESSION_USER_E
 */
uint8_t voice_compression_get_user(void);

/**
 * @brief Inform voice compression module that compression process started
 * 
 * @param user          The user request compression start
 * @param codec         The codec will be used in compression
 * @param oneShotHandleLen  PCM data length for one-shot process
 */
void voice_compression_start(uint8_t user, uint8_t codec, uint32_t oneShotHandleLen);

/**
 * @brief Inform voice compression module that compression process stopped
 * 
 * @param user          The user request compression stop
 * @param codec         The codec used in compression
 * @param forceStop     if force stop the compression or not
 */
void voice_compression_stop(uint8_t user, uint8_t codec, bool forceStop);

/**
 * @brief Deinit compression used buffer
 * 
 */
void voice_compression_dinit_buffer(void);

/**
 * @brief Compression handler
 * 
 * @param codec         Compression used codec
 * @param input         Pointer of input buffer
 * @param sampleCount   Sample count
 * @param purchasedBytes PCM data consumed bytes
 * @param isReset       is to reset the compression engine
 * @return uint32_t     Compression output data length
 */
uint32_t voice_compression_handle(uint8_t codec, uint8_t *input, uint32_t sampleCount, uint32_t *purchasedBytes, uint8_t isReset);

uint8_t voice_compression_validity_check();
uint32_t voice_compression_get_encode_buf_size();
int voice_compression_get_encoded_data(uint8_t *out_buf,uint32_t len);
int voice_compression_peek_encoded_data(uint8_t *out_buf,uint32_t len);
void voice_compression_reset_encode_buf(void);
void voice_compression_dequeue_encoded_data(uint32_t len);
uint32_t voice_compression_get_encoded_data_write_offset(void);


/**
 * @brief Get the encode count
 * 
 * @return uint32_t     encode count
 */
uint32_t voice_compression_get_encode_cnt(void);

#ifdef ALEXA_M55_ENABLED
void encodeFrameBuf_enqueue(uint8_t *dataBuf, uint32_t size);
void mcu_init_encode_data_queue(void);
#endif

#ifdef __cplusplus	
}
#endif

#endif	// __VOICE_SBC_H__

