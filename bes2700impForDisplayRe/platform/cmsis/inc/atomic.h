#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#include "cmsis_compiler.h"

#if   (defined(__ARM_ARCH_7A__) && (__ARM_ARCH_7A__ != 0))
#include "atomic_ca.h"
#else
#include "atomic_cm.h"
#endif
#endif /*__ATOMIC_H__*/
