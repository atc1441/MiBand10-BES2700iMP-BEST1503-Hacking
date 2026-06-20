#ifndef __SPEECH_2MIC_PREAF_H__
#define __SPEECH_2MIC_PREAF_H__

#include <stdint.h>
#include "custom_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t     bypass;
    int32_t     ref_delay;
    float       talk_error_threshold;
    float       ff_error_threshold;
    int32_t     talk_filter_len;
    int32_t     ff_filter_len;
} Speech2MicPreafConfig;

struct Speech2MicPreafState_;

typedef struct Speech2MicPreafState_ Speech2MicPreafState;

Speech2MicPreafState *speech_2mic_preaf_create(int32_t sample_rate, int32_t frame_size, Speech2MicPreafConfig *cfg, custom_allocator *allocator);

int32_t speech_2mic_preaf_destroy(Speech2MicPreafState *st);

int32_t speech_2mic_preaf_process(Speech2MicPreafState *st, int16_t *pcm_buf, int16_t *ref_buf, int32_t pcm_len, int16_t *out_buf);

float speech_2mic_preaf_get_required_mips(Speech2MicPreafState *st);

int32_t speech_2mic_preaf_set_config(Speech2MicPreafState *st, const Speech2MicPreafConfig *cfg);

#ifdef __cplusplus
}
#endif

#endif