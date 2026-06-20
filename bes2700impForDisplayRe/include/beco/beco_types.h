#pragma once
//
// Copyright 2020 BES Technic
//
// Description: Beco fractional types
//

#ifndef _BECO_TYPES_H
#define _BECO_TYPES_H

#include <stdint.h>
#include "beco_common.h"

BECO_C_DECLARATIONS_START

// Signed fractions
//
// Q7 type range is from -128 to 127 (divide by 128 to get fraction)
// Q15 type range is from -0x8000 to 0x7fff (divide by 0x8000 to get fraction)

// Unsigned fractions
//
// P7 type range is from 0 to 255 (128 = 1.0) (divide by 128 to get fraction)
// P15 type range is from 0 to 0xffff (0x8000 = 1.0) (divide by 0x8000 to get fraction)

#define Q7_SHIFT   7
#define Q15_SHIFT 15

typedef int32_t  q31_t;
typedef int16_t  q15_t;
typedef int8_t   q7_t;

typedef uint32_t p31_t;
typedef uint16_t p15_t;
typedef uint8_t  p7_t;

#define q_round(shift) (shift > 0? (1 << (shift-1)): 0)
#define q7_sat(v)      ((v) > INT8_MAX?  INT8_MAX:  (v) < INT8_MIN?  INT8_MIN: (v))
#define q15_sat(v)     ((v) > INT16_MAX? INT16_MAX: (v) < INT16_MIN? INT16_MIN: (v))
#define p7_sat(v)      ((v) > UINT8_MAX?  UINT8_MAX:  (v) < 0? 0: (v))
#define p15_sat(v)     ((v) > UINT16_MAX? UINT16_MAX: (v) < 0? 0: (v))

#define FLOAT_TO_Q(f, T, N) ((T) roundf(scalbnf(f, N)))

#define P7(f)   FLOAT_TO_Q(f, p7_t, Q7_SHIFT)
#define P15(f)  FLOAT_TO_Q(f, p15_t, Q15_SHIFT)
#define Q7(f)   FLOAT_TO_Q(f, q7_t, Q7_SHIFT)
#define Q15(f)  FLOAT_TO_Q(f, q15_t, Q15_SHIFT)

#define Q7_TO_FLOAT(i)  ((i)*(1.f/(INT8_MAX   + 1)))
#define Q15_TO_FLOAT(i) ((i)*(1.f/(INT16_MAX  + 1)))
#define Q31_TO_FLOAT(i) ((i)*(-1.f/INT32_MIN))  // avoid positive overflow: -1./-2147483648
#define P7_TO_FLOAT(i)  ((i)*(2.f/(UINT8_MAX  + 1)))
#define P15_TO_FLOAT(i) ((i)*(2.f/(UINT16_MAX + 1)))

BECO_C_DECLARATIONS_END

#endif
