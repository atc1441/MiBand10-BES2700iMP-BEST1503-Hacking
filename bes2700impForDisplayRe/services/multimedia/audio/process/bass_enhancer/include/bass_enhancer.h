#ifndef BASS_ENHANCER_H
#define BASS_ENHANCER_H

#include <stdint.h>

typedef struct
{
    bool switch_on;     // for AudioTool
    int low_cut_freq;   // low cut freq
    int high_cut_freq;  // high_cut_freq
    float gain0;    // invalid
    float gain1;    // gain (dB)
} BassEnhancerConfig;

struct BassEnhancerState_;

typedef struct BassEnhancerState_ BassEnhancerState;

#ifdef __cplusplus
extern "C" {
#endif

BassEnhancerState *bass_enhancer_init(int sample_rate, int frame_size, int sample_bit, int ch_num, const BassEnhancerConfig *cfg);

void bass_enhancer_destroy(BassEnhancerState *st);

void bass_enhancer_process(BassEnhancerState *st, uint8_t *buf, int pcm_len);

void bass_enhancer_switch(BassEnhancerState *st, bool onoff);

void bass_enhancer_set_cfg(BassEnhancerState *st, const BassEnhancerConfig *cfg);

#ifdef __cplusplus
}
#endif

#endif