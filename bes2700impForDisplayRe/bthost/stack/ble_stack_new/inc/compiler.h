/**
 ****************************************************************************************
 *
 * @file gcc/compiler.h
 *
 * @brief Definitions of compiler specific directives.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _COMPILER_H_
#define _COMPILER_H_

#ifndef __INLINE
#define __INLINE         inline
#endif

/// define the static keyword for this compiler
#define __STATIC static


/// define the interrupt handler attribute for this compiler
#define __IRQ

/// define the BT IRQ handler attribute for this compiler
#define __BTIRQ

/// define the BLE IRQ handler attribute for this compiler
#define __BLEIRQ

/// define the interrupt handler attribute for this compiler
#define __FIQ

/// define size of an empty array (used to declare structure with an array size not defined)
#ifndef __ARRAY_EMPTY
#define __ARRAY_EMPTY 0
#endif

/// __MODULE__ comes from the RVDS compiler that supports it
#define __MODULE__ __BASENAME_FILE__

/// Pack a structure field
#ifndef __PACKED
#define __PACKED __attribute__ ((__packed__))
#endif

/// Put a variable in a memory maintained during deep sleep
#define __LOWPOWER_SAVED

#endif // _COMPILER_H_
