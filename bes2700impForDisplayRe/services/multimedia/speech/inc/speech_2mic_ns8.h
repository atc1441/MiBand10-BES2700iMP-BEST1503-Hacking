#ifndef __SPEECH_2MIC_NS8_H__
#define __SPEECH_2MIC_NS8_H__

#include <stdint.h>
#include "custom_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t     bypass;

    float       dist;
    int32_t     min_frequency;
    int32_t     max_frequency;

    float       energy_threshold;
    float       corr_threshold;
	float       fb_ratio;
    float       crossover_threshold;
    float       Pb_energy_db;
    float       update_snr;
    int32_t     update_count;
    float       wind_mic_switch_thr;

    int32_t     wind_supp_enable;
    int32_t     echo_af_enable;
    int32_t     echo_supp_enable;
    int32_t     ref_delay;
	float       gamma;
	int32_t     echo_band_start;
	int32_t     echo_band_end;
	float       min_ovrd;

    int32_t     wdrc_enable;
    float       wdrc_CT;
    float       wdrc_CS;
    float       wdrc_ET;
    float       wdrc_ES;
    
    float       talk_pre_gain;
    float       pre_gain;
    float       post_gain;
    float       bf_ns_nowind_gain;
    float       bf_ns_wind_gain;

    int32_t     post_supp_enable;
    float       denoise_dB;
    float       dnn_denoise_dB;
} Speech2MicNs8Config;

struct Speech2MicNs8State_;

typedef struct Speech2MicNs8State_ Speech2MicNs8State;

Speech2MicNs8State *speech_2mic_ns8_create(int32_t sample_rate, int32_t frame_size, Speech2MicNs8Config *cfg, custom_allocator *allocator);

int32_t speech_2mic_ns8_destroy(Speech2MicNs8State *st);

int32_t speech_2mic_ns8_process(Speech2MicNs8State *st, int16_t *pcm_buf, int16_t *ref_buf, int32_t pcm_len, int16_t *out_buf);

float speech_2mic_ns8_get_required_mips(Speech2MicNs8State *st);

int32_t speech_2mic_ns8_set_config(Speech2MicNs8State *st, const Speech2MicNs8Config *cfg);
int32_t speech_2mic_ns8_get_delay(Speech2MicNs8State *st);

#ifdef __cplusplus
}
#endif

#endif