/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __PLAT_TYPES_H__
#define __PLAT_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"

typedef uint8_t         u8;
typedef uint16_t        u16;
typedef uint32_t        u32;
typedef uint64_t        u64;
typedef int8_t          s8;
typedef int16_t         s16;
typedef int32_t         s32;
typedef int64_t         s64;

/* IO definitions (access restrictions to peripheral registers) */
/**
    \defgroup CMSIS_glob_defs CMSIS Global Defines

    <strong>IO Type Qualifiers</strong> are used
    \li to specify the access to peripheral variables.
    \li for automatic generation of peripheral register debug information.
*/
#ifndef __I
#ifdef __cplusplus
  #define   __I     volatile             /*!< Defines 'read only' permissions                 */
#else
  #define   __I     volatile const       /*!< Defines 'read only' permissions                 */
#endif
#define     __O     volatile             /*!< Defines 'write only' permissions                */
#define     __IO    volatile             /*!< Defines 'read / write' permissions              */
#endif


#define BITFIELD_VAL(field, val)        (((val) & (field ## _MASK >> field ## _SHIFT)) << field ## _SHIFT)
#define SET_BITFIELD(reg, field, val)   (((reg) & ~field ## _MASK) | BITFIELD_VAL(field, val))
#define GET_BITFIELD(reg, field)        (((reg) & field ## _MASK) >> field ## _SHIFT)


/* Frequently used macros */

#ifndef ALIGN
#define ALIGN(val,exp)                  (((val) + ((exp)-1)) & ~((exp)-1))
#endif

#define ARRAY_SIZE(a)                   (sizeof(a)/sizeof(a[0]))
#define LAST_ELEMENT(x)                 (&x[ARRAY_SIZE(x)-1])
#define BOUND(x, min, max)              ( (x) < (min) ? (min) : ((x) > (max) ? (max):(x)) )
#define ROUND_SIZEOF(t)                 ((sizeof(t)+sizeof(int)-1)&~(sizeof(int)-1))
#define ROUND_UP(N, S)                  ((((N) + (S) - 1) / (S)) * (S))

// Structure offset
#ifndef OFFSETOF
#define OFFSETOF(TYPE, MEMBER)          ((size_t) &((TYPE *)0)->MEMBER)
#endif

#ifndef MAX
#define MAX(a,b)                        (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)                        (((a) < (b)) ? (a) : (b))
#endif
#ifndef ABS
#define ABS(x)                          ((x<0)?(-(x)):(x))
#endif

#define TO_STRING_A(s)                  # s
#define TO_STRING(s)                    TO_STRING_A(s)

/* Remove const cast-away warnings from gcc -Wcast-qual */
#define __UNCONST(a)                    ((void *)(unsigned long)(const void *)(a))

#ifdef __GNUC__

/// From http://www.ibm.com/developerworks/linux/library/l-gcc-hacks/
/// Macro to use in a if statement to tell the compiler this branch
/// is likely taken, and optimize accordingly.
#define LIKELY(x)                       __builtin_expect(!!(x), 1)
/// Macro to use in a if statement to tell the compiler this branch
/// is unlikely take, and optimize accordingly.
#define UNLIKELY(x)                     __builtin_expect(!!(x), 0)

/// For packing structure
#define PACKED                          __attribute__((packed))

/// To describe alignment
#define ALIGNED(a)                      __attribute__((aligned(a)))

/// For possibly unused functions or variables (e.g., debugging stuff)
#define POSSIBLY_UNUSED                 __attribute__((unused))

/// For functions or variables must be emitted even if not referenced
#define USED                            __attribute__((used))

/// For inline functions
#define ALWAYS_INLINE                   __attribute__((always_inline))

// For functions never inlined
#define NOINLINE                        __attribute__((noinline))

// For functions not caring performance
#ifdef __ARMCC_VERSION
#define OPT_SIZE
#else
#define OPT_SIZE                        __attribute__((optimize("Os")))
#endif

// For functions not returning
#ifndef NORETURN
#define NORETURN                        __attribute__((noreturn))
#endif

// For ASM functions in C
#ifdef __arm__
#define NAKED                           __attribute__((naked))
#else
#define NAKED                           __attribute__((error("Unsupport naked functions")))
#endif

// For weak symbols
#define WEAK                            __attribute__((weak))

/**
 * CONTAINER_OF - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#ifndef CONTAINER_OF
#if __cplusplus >= 201103L
#define CONTAINER_OF(ptr, type, member) ({          \
        const decltype(((type *)0)->member)*__mptr = (ptr);    \
    (type *)((char *)__mptr - OFFSETOF(type, member)); })
#else
#define CONTAINER_OF(ptr, type, member) ({          \
        const typeof(((type *)0)->member)*__mptr = (ptr);    \
    (type *)((char *)__mptr - OFFSETOF(type, member)); })
#endif 
#endif

#else // Not GCC

#define LIKELY(x)
#define UNLIKELY(x)
#define PACKED
#define ALIGNED(a)
#define POSSIBLY_UNUSED
#define USED
#define ALWAYS_INLINE
#define NOINLINE
#define OPT_SIZE
#define NORETURN
#define NAKED
#define WEAK

#define CONTAINER_OF(ptr, type, member) (   \
    (type *)((char *)(ptr) - OFFSETOF(type, member)))

#endif // Not GCC

/// C preprocessor conditional check
/// --------------------------------
#define GCC_VERSION (__GNUC__ * 10000 \
                       + __GNUC_MINOR__ * 100 \
                       + __GNUC_PATCHLEVEL__)

#if defined(__GNUC__) && (GCC_VERSION >= 40600) && !defined(__cplusplus)

// GCC 4.6 or later
#define STATIC_ASSERT(e, m)             _Static_assert(e, m)

#elif defined(__GNUC__) && (GCC_VERSION >= 40300) && defined(__cplusplus) && (__cplusplus >= 201103L)

#define STATIC_ASSERT(e, m)             static_assert(e, m)

#else // No built-in static assert

/// FROM: http://www.pixelbeat.org/programming/gcc/static_assert.html
#define ASSERT_CONCAT_(a, b)            a##b
#define ASSERT_CONCAT(a, b)             ASSERT_CONCAT_(a, b)
/* These can't be used after statements in c89. */
#ifdef __COUNTER__
#define STATIC_ASSERT(e, m) \
    enum { ASSERT_CONCAT(static_assert_, __COUNTER__) = 1/(!!(e)) };
#else
/* This can't be used twice on the same line so ensure if using in headers
 * that the headers are not included twice (by wrapping in #ifndef...#endif)
 * Note it doesn't cause an issue when used on same line of separate modules
 * compiled with gcc -combine -fwhole-program.  */
#define STATIC_ASSERT(e, m) \
    enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) };
#endif

#endif // No built-in static assert
/// --------------------------------

#ifdef __cplusplus
}
#endif

#endif
