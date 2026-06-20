/***************************************************************************
 *
 * Copyright 2015-2024 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#pragma once

// Description: beco coprocessor instructions bindings

typedef enum
{
    BECO_OK             =  0,        /**< No error */
    BECO_ARGUMENT_ERROR = -1,        /**< One or more arguments are incorrect */
    BECO_LENGTH_ERROR   = -2,        /**< Length of data buffer is incorrect */
    BECO_SIZE_MISMATCH  = -3,        /**< Size of matrices is not compatible with the operation */
    BECO_NANINF         = -4,        /**< Not-a-number (NaN) or infinity is generated */
    BECO_SINGULAR       = -5,        /**< Input matrix is singular and cannot be inverted */
} beco_state;


#ifdef __cplusplus
#define BECO_C_DECLARATIONS_START extern "C" {
#define BECO_C_DECLARATIONS_END   }
#else
#define BECO_C_DECLARATIONS_START
#define BECO_C_DECLARATIONS_END
#endif

#ifndef __arm__
#  define BECO_SIM
#endif

// Define a BECO_INLINE for functions that are always inlined, taking the address
// to the function return an error.
#ifndef BECO_INLINE
#  define BECO_INLINE extern inline __attribute__ ((__always_inline__, __gnu_inline__, __artificial__))
#endif

// Define BECO_ASM and BECO_VASM for inline ARM assembly code
#ifndef BECO_ASM
#  define BECO_VASM   __asm volatile
#  define BECO_ASM    __asm
#endif


// Configuration of Beco co-processor number
//
// Default  location for Beco is CPNUM 0 (_CPNUM)
// In some cases HW might implement 2 instances of Beco
// on different CPNUM, use the config variables
//
//   HW_CPU_BECO_CPNUM      - default CPNUM for all instructions
//   HW_CPU_BECO_CPNUM_FFT  - override of CPNUM only for FFT instructions
//
// 2005: Have a bug in the matrix multiply (the signed sum-pack bug), this chip
//       have two beco. Use config to use beco-fft only for FFT instructions:
//       HW_CPU_BECO_CPNUM     0
//       HW_CPU_BECO_CPNUM_FFT 1
//
#ifdef HW_CPU_BECO_CPNUM
#undef  _CPNUM
#define _CPNUM       HW_CPU_BECO_CPNUM
#endif

#ifdef HW_CPU_BECO_CPNUM_FFT
#undef  _CPNUM_FFT
#define _CPNUM_FFT   HW_CPU_BECO_CPNUM_FFT
#endif

#ifndef _CPNUM
#define _CPNUM      0
#endif
#ifndef _CPNUM_FFT
#define _CPNUM_FFT  _CPNUM
#endif

#ifndef BECO_USE_ASM_BINDINGS
#  if __clang__
//    clang's builtin_* implementation is seriously broken.
#     define BECO_USE_ASM_BINDINGS 1 // Must use ASM bindings
#  elif __GNUC__
//    gcc can use the coprocessor instructions bindings from arm_acle.h,
//    but we prefer same asm bindings as used for clang.
#     define BECO_USE_ASM_BINDINGS 1 // Can use ASM bindings or acle builtin bindings
#  else
#     define BECO_USE_ASM_BINDINGS 0 // Can not use ASM bindings, hopefully acle work
#  endif
#endif

// Select beco bindings
#ifdef BECO_SIM
#elif (BECO_USE_ASM_BINDINGS == 1)
#  include "coproc-if.h"  // asm
#else
#  include "beco-if.h"    // acle
#endif

// Make 'restrict' avaliable
#ifndef restrict
#  define restrict __restrict
#endif


// OS / SIMULATOR HOOK

int beco_init(void);
int beco_exit(void);

#ifndef BECO_INIT
#  ifndef BECO_NO_SYSTEM_INIT
#    define BECO_INIT()    beco_init()
#    define BECO_EXIT(ret) beco_exit()
#  else
#    define BECO_INIT()
#    define BECO_EXIT(ret)
#  endif
#endif

