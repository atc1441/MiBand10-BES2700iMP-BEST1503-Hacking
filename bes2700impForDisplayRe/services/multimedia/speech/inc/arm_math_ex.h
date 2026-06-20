#ifndef ARM_MATH_EX_H
#define ARM_MATH_EX_H

#include "arm_math.h"

typedef int32_t q23_t;

#ifdef __cplusplus
extern "C" {
#endif

void arm_q15_to_q23(const q15_t * pSrc, q23_t * pDst, uint32_t blockSize);

void arm_q23_to_q15(const q23_t * pSrc, q15_t * pDst, uint32_t blockSize);

void arm_conj_cmplx_mult_cmplx_f32(
  const float32_t * pSrcA,
  const float32_t * pSrcB,
        float32_t * pDst,
        uint32_t numSamples);

#ifdef __cplusplus
}
#endif

#endif