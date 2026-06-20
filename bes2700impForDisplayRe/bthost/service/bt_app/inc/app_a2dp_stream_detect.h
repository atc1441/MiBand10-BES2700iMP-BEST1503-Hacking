#ifndef __APP_A2DP_STREAM_DETECT_H_
#define __APP_A2DP_STREAM_DETECT_H_

#if defined(A2DP_STREAM_DETECT_NO_DECODE)

#include "bluetooth.h"

typedef enum {
    MUTE_STREAM,
    NORMAL_STREAM,
    UNKNOWN_STREAM,
} stream_t;

typedef enum {
    SBC_CODEC,
    AAC_CODEC,
    UNKNOWN_CODEC,
} codec_type_t;

typedef struct {
    stream_t result;
    uint32_t sample_count;
} detect_result_t;

typedef struct {
    uint8_t sbc_detect_threshold;
    uint8_t aac_detect_threshold;
} stream_detct_param_t;

detect_result_t app_a2dp_detect_sbc_stream(U8 * media_payload);

#ifdef A2DP_AAC_ON
detect_result_t app_a2dp_detect_aac_stream(U8 * media_payload, uint8_t media_payload_len);
#endif // A2DP_AAC_ON

void app_a2dp_stream_set_detect_threshold(codec_type_t codec, uint8_t threshold);

#endif // A2DP_STREAM_DETECT_NO_DECODE
#endif // __APP_A2DP_STREAM_DETECT_H_