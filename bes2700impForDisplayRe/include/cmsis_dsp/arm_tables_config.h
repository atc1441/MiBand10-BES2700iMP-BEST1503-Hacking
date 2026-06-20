
/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_common_tables.h
 * Description:  Extern declaration for common tables
 *
 * $Date:        27. January 2017
 * $Revision:    V.1.5.1
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2017 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ARM_TABLES_CONFIG_H
#define _ARM_TABLES_CONFIG_H

#ifdef   __cplusplus
extern "C"
{
#endif

#define ARM_DSP_CONFIG_TABLES

// #define ARM_ALL_FFT_TABLES

#define ARM_FFT_ALLOW_TABLES

#if defined(ARM_DSP_CONFIG_TABLES) && !defined(ARM_ALL_FFT_TABLES)

  /* Double Precision Float CFFT twiddles */
// #define ARM_TABLE_BITREV_1024

// #define ARM_TABLE_TWIDDLECOEF_F64_16

// #define ARM_TABLE_TWIDDLECOEF_F64_32

// #define ARM_TABLE_TWIDDLECOEF_F64_64

// #define ARM_TABLE_TWIDDLECOEF_F64_128

// #define ARM_TABLE_TWIDDLECOEF_F64_256

// #define ARM_TABLE_TWIDDLECOEF_F64_512

// #define ARM_TABLE_TWIDDLECOEF_F64_1024

// #define ARM_TABLE_TWIDDLECOEF_F64_2048

// #define ARM_TABLE_TWIDDLECOEF_F64_4096

// #define ARM_TABLE_TWIDDLECOEF_F32_16

// #define ARM_TABLE_TWIDDLECOEF_F32_32

#define ARM_TABLE_TWIDDLECOEF_F32_64

#define ARM_TABLE_TWIDDLECOEF_F32_128

#define ARM_TABLE_TWIDDLECOEF_F32_256

// #define ARM_TABLE_TWIDDLECOEF_F32_512

// #define ARM_TABLE_TWIDDLECOEF_F32_1024

// #define ARM_TABLE_TWIDDLECOEF_F32_2048

// #define ARM_TABLE_TWIDDLECOEF_F32_4096

  /* Q31 */

// #define ARM_TABLE_TWIDDLECOEF_Q31_16

// #define ARM_TABLE_TWIDDLECOEF_Q31_32

// #define ARM_TABLE_TWIDDLECOEF_Q31_64

// #define ARM_TABLE_TWIDDLECOEF_Q31_128

// #define ARM_TABLE_TWIDDLECOEF_Q31_256

// #define ARM_TABLE_TWIDDLECOEF_Q31_512

// #define ARM_TABLE_TWIDDLECOEF_Q31_1024

// #define ARM_TABLE_TWIDDLECOEF_Q31_2048

// #define ARM_TABLE_TWIDDLECOEF_Q31_4096

// #define ARM_TABLE_TWIDDLECOEF_Q15_16

// #define ARM_TABLE_TWIDDLECOEF_Q15_32

// #define ARM_TABLE_TWIDDLECOEF_Q15_64

#define ARM_TABLE_TWIDDLECOEF_Q15_128

#define ARM_TABLE_TWIDDLECOEF_Q15_256

// #define ARM_TABLE_TWIDDLECOEF_Q15_512

// #define ARM_TABLE_TWIDDLECOEF_Q15_1024

// #define ARM_TABLE_TWIDDLECOEF_Q15_2048

// #define ARM_TABLE_TWIDDLECOEF_Q15_4096

  /* Double Precision Float RFFT twiddles */
// #define ARM_TABLE_TWIDDLECOEF_RFFT_F64_32

// #define ARM_TABLE_TWIDDLECOEF_RFFT_F64_64

// #define ARM_TABLE_TWIDDLECOEF_RFFT_F64_128

// #define ARM_TABLE_TWIDDLECOEF_RFFT_F64_256

// #define ARM_TABLE_TWIDDLECOEF_RFFT_F64_512

// #define ARM_TABLE_TWIDDLECOEF_RFFT_F64_1024

// #define ARM_TABLE_TWIDDLECOEF_RFFT_F64_2048

// #define ARM_TABLE_TWIDDLECOEF_RFFT_F64_4096


// #define ARM_TABLE_TWIDDLECOEF_RFFT_F32_32

// #define ARM_TABLE_TWIDDLECOEF_RFFT_F32_64

#define ARM_TABLE_TWIDDLECOEF_RFFT_F32_128

#define ARM_TABLE_TWIDDLECOEF_RFFT_F32_256

#define ARM_TABLE_TWIDDLECOEF_RFFT_F32_512

// #define ARM_TABLE_TWIDDLECOEF_RFFT_F32_1024

// #define ARM_TABLE_TWIDDLECOEF_RFFT_F32_2048

// #define ARM_TABLE_TWIDDLECOEF_RFFT_F32_4096

  /* Double precision floating-point bit reversal tables */

// #define ARM_TABLE_BITREVIDX_FLT64_16

// #define ARM_TABLE_BITREVIDX_FLT64_32

// #define ARM_TABLE_BITREVIDX_FLT64_64

// #define ARM_TABLE_BITREVIDX_FLT64_128

// #define ARM_TABLE_BITREVIDX_FLT64_256

// #define ARM_TABLE_BITREVIDX_FLT64_512

// #define ARM_TABLE_BITREVIDX_FLT64_1024

// #define ARM_TABLE_BITREVIDX_FLT64_2048

// #define ARM_TABLE_BITREVIDX_FLT64_4096

  /* floating-point bit reversal tables */

// #define ARM_TABLE_BITREVIDX_FLT_16

// #define ARM_TABLE_BITREVIDX_FLT_32

#define ARM_TABLE_BITREVIDX_FLT_64

#define ARM_TABLE_BITREVIDX_FLT_128

#define ARM_TABLE_BITREVIDX_FLT_256

// #define ARM_TABLE_BITREVIDX_FLT_512

// #define ARM_TABLE_BITREVIDX_FLT_1024

// #define ARM_TABLE_BITREVIDX_FLT_2048

// #define ARM_TABLE_BITREVIDX_FLT_4096

  /* fixed-point bit reversal tables */

// #define ARM_TABLE_BITREVIDX_FXT_16

// #define ARM_TABLE_BITREVIDX_FXT_32

#define ARM_TABLE_BITREVIDX_FXT_64

#define ARM_TABLE_BITREVIDX_FXT_128

#define ARM_TABLE_BITREVIDX_FXT_256

// #define ARM_TABLE_BITREVIDX_FXT_512

// #define ARM_TABLE_BITREVIDX_FXT_1024

// #define ARM_TABLE_BITREVIDX_FXT_2048

// #define ARM_TABLE_BITREVIDX_FXT_4096

// #define ARM_TABLE_REALCOEF_F32

// #define ARM_TABLE_REALCOEF_Q31

#define ARM_TABLE_REALCOEF_Q15

// #define ARM_TABLE_DCT4_F32_128

// #define ARM_TABLE_DCT4_F32_512

// #define ARM_TABLE_DCT4_F32_2048

// #define ARM_TABLE_DCT4_F32_8192

// #define ARM_TABLE_DCT4_Q15_128

// #define ARM_TABLE_DCT4_Q15_512

// #define ARM_TABLE_DCT4_Q15_2048

// #define ARM_TABLE_DCT4_Q15_8192

// #define ARM_TABLE_DCT4_Q31_128

// #define ARM_TABLE_DCT4_Q31_512

// #define ARM_TABLE_DCT4_Q31_2048

#endif /* defined(ARM_DSP_CONFIG_TABLES) && !defined(ARM_FFT_ALLOW_TABLES) */

/* fast tables */
#define ARM_FAST_ALLOW_TABLES

#if defined(ARM_DSP_CONFIG_TABLES) && !defined(ARM_ALL_FAST_TABLES)

#define ARM_TABLE_RECIP_Q15

#define ARM_TABLE_RECIP_Q31

  /* Tables for Fast Math Sine and Cosine */
#define ARM_TABLE_SIN_F32

#define ARM_TABLE_SIN_Q31

#define ARM_TABLE_SIN_Q15

#if defined(ARM_MATH_MVEI)
# define ARM_TABLE_FAST_SQRT_Q31_MVE
#endif

#if defined(ARM_MATH_MVEI)
# define ARM_TABLE_FAST_SQRT_Q15_MVE
#endif

#endif /* #if defined(ARM_DSP_CONFIG_TABLES) && !defined(ARM_FAST_ALLOW_TABLES) */


// #define ARM_MATH_MVEF

// #define ARM_MATH_HELIUM

// #define ARM_MATH_AUTOVECTORIZE

#ifdef   __cplusplus
}
#endif

#endif /*  _ARM_TABLES_CONFIG_H*/
