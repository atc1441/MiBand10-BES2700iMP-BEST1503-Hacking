#ifndef FREQZ_H
#define FREQZ_H

#ifdef __cplusplus
extern "C" {
#endif

void freqz(float *b, int M, float *a, int N, float *h, int nfft, float *work_buffer);

void freqz_fir(float* b, int M, float* h, int nfft);

// output should be fft_lenght / 2 + 1
void freqz_mag(float *h, int nfft, float *mag);

// output should be fft_lenght / 2 + 1
void freqz_angle(float *h, int nfft, float *angle);

float find_peak(float *mag, int len, float *idx);

#ifdef __cplusplus
}
#endif
#endif