#ifndef __SPEECH_2MIC_NS7_H__
#define __SPEECH_2MIC_NS7_H__

#include <stdint.h>
#include "custom_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    int32_t     bypass;
    int32_t     vad_bin_start1;
    int32_t     vad_bin_end1;
    int32_t     vad_bin_start2;
    int32_t     vad_bin_end2;
    int32_t     vad_bin_start3;
    int32_t     vad_bin_end3;
    float       coef1_thd;
    float       coef2_thd;
    float       coef3_thd;
    int32_t     supp_times;

    int32_t     calib_enable;
    int32_t     calib_delay;
    float*      filter;
    int32_t     filter_len;

    int32_t     wind_supp_enable;
    int32_t     echo_supp_enable;
    int32_t     post_supp_enable;
    int32_t     wdrc_enable;

    float       pre_gain;
    float       post_gain;
    float       denoise_dB;
    float       dnn_denoise_dB;
} Speech2MicNs7Config;

struct Speech2MicNs7State_;
typedef struct Speech2MicNs7State_ Speech2MicNs7State;

Speech2MicNs7State *speech_2mic_ns7_create(int32_t sample_rate, int32_t frame_size, Speech2MicNs7Config *cfg, custom_allocator *allocator);
int32_t speech_2mic_ns7_destroy(Speech2MicNs7State *st);
int32_t speech_2mic_ns7_process(Speech2MicNs7State *st, int16_t *pcm_buf, int16_t *ref_buf, int32_t pcm_len, int16_t *out_buf);
float speech_2mic_ns7_get_required_mips(Speech2MicNs7State *st);
#ifdef __cplusplus
}
#endif

#endif
