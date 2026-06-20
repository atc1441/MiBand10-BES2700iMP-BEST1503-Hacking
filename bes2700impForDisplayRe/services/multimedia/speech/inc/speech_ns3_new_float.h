
#ifndef __SPEECH_NS3_NEW_FLOAT_H__
#define __SPEECH_NS3_NEW_FLOAT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int32_t     bypass;
    float       denoise_dB;
} SpeechNs3FloatConfig;

struct SpeechNs3FloatState_;
typedef struct NsHandleT NsHandle;

typedef struct SpeechNs3FloatState_ SpeechNs3FloatState;

SpeechNs3FloatState *speech_ns3float_create(int32_t sample_rate, int32_t frame_size, const SpeechNs3FloatConfig *cfg);
int32_t speech_ns3float_destroy(SpeechNs3FloatState *st);
//int32_t speech_ns3float_set_config(SpeechNs3FloatState *st, const SpeechNs3FloatConfig *cfg);
int32_t speech_ns3float_process(SpeechNs3FloatState *st, int16_t *pcm_buf, int32_t pcm_len);

#ifdef __cplusplus
}
#endif

#endif
