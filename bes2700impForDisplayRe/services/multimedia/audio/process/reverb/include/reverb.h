#ifndef AUDIO_REVERB_H
#define AUDIO_REVERB_H

#include <stdint.h>

typedef struct {
    int32_t bypass; // bool; 0 - enable, 1 - disable, default = 0
    int32_t high_pass_f0; // int32; 50 ~ 1000， default = 120
    int32_t gain; // int32; -99 ~ 12, default = 0
} ReverbConfig;

struct ReverbState_;

typedef struct ReverbState_ ReverbState;

#ifdef __cplusplus
extern "C" {
#endif

ReverbState *reverb_create(int32_t sample_rate, int32_t frame_size, int32_t sample_bits, int32_t ch_num, const ReverbConfig *cfg);

void reverb_destroy(ReverbState *st);

void reverb_set_config(ReverbState *st, const ReverbConfig *cfg);

void reverb_preprocess(ReverbState *st, const uint8_t *buf, uint32_t len);

void reverb_process(ReverbState *st, uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif
