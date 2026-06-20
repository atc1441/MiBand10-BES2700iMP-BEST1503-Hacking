
#ifndef __SPEECH_NS4_H__
#define __SPEECH_NS4_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t     bypass;
    float       denoise_dB;
} SpeechNs4Config;

struct SpeechNs4State_;

typedef struct SpeechNs4State_ SpeechNs4State;

SpeechNs4State *speech_ns4_create(int32_t sample_rate, int32_t frame_size, const SpeechNs4Config *cfg);
int32_t speech_ns4_destroy(SpeechNs4State *st);
int32_t speech_ns4_process(SpeechNs4State *st, int16_t *pcm_buf, int32_t pcm_len);

#ifdef __cplusplus
}
#endif

#endif
