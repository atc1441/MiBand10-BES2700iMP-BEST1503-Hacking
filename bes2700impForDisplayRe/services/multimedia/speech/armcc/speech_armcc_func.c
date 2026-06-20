/***************************************************************************
*
* Copyright 2015-2022 BES.
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
/**
* Customer use armcc compiler to generate their lib and integrate into sdk.
* We use gcc to build sdk, perhaps you can find the build error as following: "undefined reference to `__hardfp_logf'"
* The reason is gcc and armcc generate different names for standard math lib. So we need to do a wrap for armcc function
*/
#include "math.h"

float __hardfp_logf (float x) { return logf(x); }
float __hardfp_expf (float x) { return expf(x); }
double __hardfp_exp2 (double x) { return exp2(x); }
float __hardfp_exp2f (float x) { return exp2f(x); }
float __hardfp_powf (float x, float y) { return powf(x, y); }
float __hardfp_log10f (float x) { return log10f(x); }
float __hardfp_sinf (float x) { return sinf(x); }
float __hardfp_cosf (float x) { return cosf(x); }
double __hardfp_sin (double x) { return sin(x); }
double __hardfp_cos (double x) { return cos(x); }
float __hardfp_frexpf (float x, int *y) { return frexpf(x,y); }
double __hardfp_sqrt (double x) { return sqrt(x); }
double __hardfp_floor (double x) { return floor(x); }

// Add more math function
/*__attribute__((weak,noreturn))
void __aeabi_assert (const char *expr, const char *file, int line)
{
    __assert(file, line, expr);
}*/

/* FIXME:rm this for nuttx compile*/
#if defined(__NuttX__)
#include <string.h>
__attribute__((weak))
void __aeabi_memmove (void *dest, const void *source, size_t n)
{
    memmove (dest, source, n);
}

void __aeabi_memclr(void *ptr, size_t size)
{
    memset(ptr, 0, size);
}

void __aeabi_memclr4 (void *dest, size_t n)
	_ATTRIBUTE ((alias ("__aeabi_memclr")));

void __aeabi_memclr8 (void *dest, size_t n)
	_ATTRIBUTE ((alias ("__aeabi_memclr")));


void __aeabi_memcpy(void *dest, const void *source, size_t n)
{
    // extern void memcpy (void *dest, const void *source, size_t n);
    memcpy (dest, source, n);
}

void __aeabi_memcpy4(void *dest, const void *source, size_t n)
	_ATTRIBUTE ((alias ("__aeabi_memcpy")));

void __aeabi_memcpy8(void *dest, const void *source, size_t n)
	_ATTRIBUTE ((alias ("__aeabi_memcpy")));

void __aeabi_memmove4(void *dest, const void *source, size_t n)
	_ATTRIBUTE ((alias ("__aeabi_memmove")));
#endif
// Add other module