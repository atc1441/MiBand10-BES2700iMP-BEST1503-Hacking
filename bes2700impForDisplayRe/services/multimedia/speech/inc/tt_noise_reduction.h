#ifndef TT_NOISE_REDUCTION_H
#define TT_NOISE_REDUCTION_H

#include <stdint.h>
#include "custom_allocator.h"

#define TT_NS_TAPS (40)
// #define FREQ_IMPL

typedef struct
{
    float alpha;
    int32_t denoise_dB;
    float noise_psd_dB[TT_NS_TAPS + 1];
} TtNsConfig;

struct TtNsState_;

typedef struct TtNsState_ TtNsState;

typedef int32_t (*tt_ns_callback)(float *coeffs, uint32_t num);

TtNsState *tt_ns_create(int32_t sample_rate, int32_t frame_size, TtNsConfig *cfg, custom_allocator *allocator);

void tt_ns_destroy(TtNsState *st);

void tt_ns_process(TtNsState *st, float *pcm_buf, uint32_t pcm_len, tt_ns_callback cb);

void tt_ns_process_q23(TtNsState *st, int32_t *pcm_buf, uint32_t pcm_len, tt_ns_callback cb);

void tt_ns_set_cfg(TtNsState *st, TtNsConfig *cfg);

#endif
