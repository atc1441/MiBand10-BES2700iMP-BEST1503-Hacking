#ifndef __SPEECH_3MIC_NS2_H__
#define __SPEECH_3MIC_NS2_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
	int32_t     bypass;
	int32_t     wnr_enable;
	float       denoise_dB;
	float       delay_taps;
	short       freq_smooth_enable;
	int32_t     crossover_freq;
} Speech3MicNs2Config;
struct Speech3MicNs2State_;
typedef struct Speech3MicNs2State_ Speech3MicNs2State;
Speech3MicNs2State *speech_3mic_ns2_create(int sample_rate, int frame_size, Speech3MicNs2Config *cfg);
int32_t speech_3mic_ns2_destroy(Speech3MicNs2State *st);
int32_t speech_3mic_ns2_process(Speech3MicNs2State *st, short *pcm_buf, short *ref_buf, int32_t pcm_len, short *out_buf);
int speech_3mic_ns2_set_config(Speech3MicNs2State *st, const Speech3MicNs2Config *cfg);
float speech_3mic_ns2_get_required_mips(Speech3MicNs2State *st);
#ifdef __cplusplus
}
#endif

#endif