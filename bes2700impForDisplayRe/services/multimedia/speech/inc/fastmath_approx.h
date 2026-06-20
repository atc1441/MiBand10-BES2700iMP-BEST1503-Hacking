#ifndef FASTMATH_APPROX_H
#define FASTMATH_APPROX_H

#ifdef __cplusplus
extern "C" {
#endif

float pow2f_approx(float x);

float pow10f_approx(float x);

float expf_approx(float x);

float powf_approx(float x, float y);

float log2f_approx(float x);

float log10f_approx(float x);

float logf_approx(float x);

#ifdef __cplusplus
}
#endif

#endif
