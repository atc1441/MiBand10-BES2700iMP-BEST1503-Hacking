#ifndef DTLN_DENOISE_H
#define DTLN_DENOISE_H

#include <stdint.h>
#include "arm_math.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct DTLNStftState_s DTLNStftState_t;
typedef struct DTLNModelState_s DTLNModelState_t;

DTLNStftState_t* stft_create(uint32_t sample_rate, uint32_t frame_size, bool cp);

void stft_destroy(DTLNStftState_t *st);

DTLNModelState_t* model_create();

void model_destroy(DTLNModelState_t *st);

int32_t dtln_denoise_process(DTLNStftState_t *stft_st, DTLNModelState_t *model_st, int16_t *pcm_buf, uint32_t pcm_len);

int32_t dtln_denoise_process_frames(DTLNStftState_t *stft_st, DTLNModelState_t *model_st, int16_t *pcm_buf, uint32_t pcm_len);

float* dtln_denoise_process_spectrum(DTLNStftState_t *stft_st, DTLNModelState_t *model_st, float* ft, int32_t nfft);

float* dtln_denoise_noise_estimate(DTLNStftState_t* stft_st);

float* dtln_denoise_snr(DTLNStftState_t* stft_st);

#ifdef __cplusplus
}
#endif

#endif
