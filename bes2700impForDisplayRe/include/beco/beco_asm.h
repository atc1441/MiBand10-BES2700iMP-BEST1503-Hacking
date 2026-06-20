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

#ifndef _BECO_ASM_H
#define _BECO_ASM_H

#include <stdint.h>
#include "beco_common.h"

BECO_C_DECLARATIONS_START

#ifdef __arm__
#define BECO_VASM   __asm volatile
#define BECO_ASM    __asm


#define BECO_SSAT(ARG1, ARG2) \
__extension__ \
({                          \
  int32_t __RES, __ARG1 = (ARG1); \
  BECO_ASM ("ssat %0, %1, %2" : "=r" (__RES) :  "I" (ARG2), "r" (__ARG1) : "cc" ); \
  __RES; \
 })


#define BECO_USAT(ARG1, ARG2) \
 __extension__ \
({                          \
  uint32_t __RES, __ARG1 = (ARG1); \
  BECO_ASM ("usat %0, %1, %2" : "=r" (__RES) :  "I" (ARG2), "r" (__ARG1) : "cc" ); \
  __RES; \
 })


#define BECO_SMLAD(OP1, OP2, OP3) \
 __extension__                    \
({                                \
  int32_t result;                 \
  uint32_t op1 = (OP1), op2 = (OP2), op3 = (OP3);   \
  BECO_ASM ("smlad %0, %1, %2, %3" : "=r" (result) : "r" (op1), "r" (op2), "r" (op3) ); \
  result; \
})

#define BECO_SXTB16(OP1, rotate)  \
 __extension__                    \
({                                \
  uint32_t op1 = (OP1), result;   \
  BECO_VASM ("sxtb16 %0, %1, ROR %2" : "=r" (result) : "r" (op1), "i" (rotate) ); \
  result;                         \
})

#define BECO_PKHBT(ARG1,ARG2,ARG3) \
({                          \
  uint32_t __RES, __ARG1 = (ARG1), __ARG2 = (ARG2); \
  BECO_ASM ("pkhbt %0, %1, %2, lsl %3" : "=r" (__RES) :  "r" (__ARG1), "r" (__ARG2), "I" (ARG3)  ); \
  __RES; \
 })

#define BECO_PKHTB(ARG1,ARG2,ARG3) \
({                          \
  uint32_t __RES, __ARG1 = (ARG1), __ARG2 = (ARG2); \
  BECO_ASM ("pkhtb %0, %1, %2, asr %3" : "=r" (__RES) :  "r" (__ARG1), "r" (__ARG2), "I" (ARG3)  ); \
  __RES; \
 })


#else

inline int32_t BECO_SSAT(int32_t val, uint32_t sat)
{
  if ((sat >= 1U) && (sat <= 32U))
  {
    const int32_t max = (int32_t)((1U << (sat - 1U)) - 1U);
    const int32_t min = -1 - max;

    return (val > max)? max : (val < min)? min: val;
  }
  return val;
}

inline uint32_t BECO_USAT(int32_t val, uint32_t sat)
{
  if (sat <= 31U)
  {
    const uint32_t max = ((1U << sat) - 1U);

    if (val > (int32_t)max) return max;
    else if (val < 0)       return 0U;
  }
  return (uint32_t)val;
}

inline int32_t BECO_SMLAD(uint32_t op1, uint32_t op2, int32_t acc)
{
  union { uint32_t v; int16_t x[2]; } a, b;
  a.v = op1;
  b.v = op2;
  return  acc + a.x[1]*b.x[1] + a.x[0]*b.x[0];
}

inline uint32_t BECO_SXTB16(uint32_t op1, uint32_t rotate)
{
  uint32_t a = (uint16_t)(int8_t)(op1 >> rotate);
  uint32_t b = (uint16_t)(int8_t)(op1 >> (rotate+16));
  return (b << 16) | a;
}


#define BECO_PKHBT(ARG1,ARG2,ARG3)         ( ((((uint32_t)(ARG1))          ) & 0x0000FFFFUL) |  \
                                           ((((uint32_t)(ARG2)) << (ARG3)) & 0xFFFF0000UL)  )

#define BECO_PKHTB(ARG1,ARG2,ARG3)         ( ((((uint32_t)(ARG1))          ) & 0xFFFF0000UL) |  \
                                           ((((uint32_t)(ARG2)) >> (ARG3)) & 0x0000FFFFUL)  )

#endif

BECO_C_DECLARATIONS_END

#endif
