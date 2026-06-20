/*
 * Copyright (c) 2013-2020 Arm Limited. All rights reserved.
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
 *
 * -----------------------------------------------------------------------------
 *
 * Project:     CMSIS-RTOS RTX
 * Title:       Cortex-M Core definitions
 *
 * -----------------------------------------------------------------------------
 */

#ifndef RTX_CORE_CM_H_
#define RTX_CORE_CM_H_

#ifndef RTX_CORE_C_H_
#include "RTE_Components.h"
#include CMSIS_device_header
#endif

#include <stdbool.h>
typedef bool bool_t;

#ifndef FALSE
#define FALSE                   ((bool_t)0)
#endif

#ifndef TRUE
#define TRUE                    ((bool_t)1)
#endif

#ifdef  RTE_CMSIS_RTOS2_RTX5_ARMV8M_NS
#define DOMAIN_NS               1
#endif

#ifndef DOMAIN_NS
#define DOMAIN_NS               0
#endif

#if    (DOMAIN_NS == 1)
#if   ((!defined(__ARM_ARCH_8M_BASE__)   || (__ARM_ARCH_8M_BASE__   == 0)) && \
       (!defined(__ARM_ARCH_8M_MAIN__)   || (__ARM_ARCH_8M_MAIN__   == 0)) && \
       (!defined(__ARM_ARCH_8_1M_MAIN__) || (__ARM_ARCH_8_1M_MAIN__ == 0)))
#error "Non-secure domain requires ARMv8-M Architecture!"
#endif
#endif

#ifndef EXCLUSIVE_ACCESS
#if   ((defined(__ARM_ARCH_7M__)        && (__ARM_ARCH_7M__        != 0)) || \
       (defined(__ARM_ARCH_7EM__)       && (__ARM_ARCH_7EM__       != 0)) || \
       (defined(__ARM_ARCH_8M_BASE__)   && (__ARM_ARCH_8M_BASE__   != 0)) || \
       (defined(__ARM_ARCH_8M_MAIN__)   && (__ARM_ARCH_8M_MAIN__   != 0)) || \
       (defined(__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ != 0)))
#define EXCLUSIVE_ACCESS        1
#else
#define EXCLUSIVE_ACCESS        0
#endif
#endif

#define OS_TICK_HANDLER         SysTick_Handler

/// xPSR_Initialization Value
/// \param[in]  privileged      true=privileged, false=unprivileged
/// \param[in]  thumb           true=Thumb, false=ARM
/// \return                     xPSR Init Value
__STATIC_INLINE uint32_t xPSR_InitVal (bool_t privileged, bool_t thumb) {
  (void)privileged;
  (void)thumb;
  return (0x01000000U);
}

// Stack Frame:
//  - Extended: S16-S31, R4-R11, R0-R3, R12, LR, PC, xPSR, S0-S15, FPSCR
//  - Basic:             R4-R11, R0-R3, R12, LR, PC, xPSR

/// Stack Frame Initialization Value (EXC_RETURN[7..0])
#if (DOMAIN_NS == 1)
#define STACK_FRAME_INIT_VAL    0xBCU
#else
#define STACK_FRAME_INIT_VAL    0xFDU
#endif

/// Stack Offset of Register R0
/// \param[in]  stack_frame     Stack Frame (EXC_RETURN[7..0])
/// \return                     R0 Offset
__STATIC_INLINE uint32_t StackOffsetR0 (uint8_t stack_frame) {
#if ((__FPU_USED == 1U) || \
     (defined(__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE > 0)))
  return (((stack_frame & 0x10U) == 0U) ? ((16U+8U)*4U) : (8U*4U));
#else
  (void)stack_frame;
  return (8U*4U);
#endif
}


//  ==== Core functions ====

//lint -sem(__get_CONTROL, pure)
//lint -sem(__get_IPSR,    pure)
//lint -sem(__get_PRIMASK, pure)
//lint -sem(__get_BASEPRI, pure)

/// Check if running Privileged
/// \return     true=privileged, false=unprivileged
__STATIC_INLINE bool_t IsPrivileged (void) {
  return ((__get_CONTROL() & 1U) == 0U);
}

/// Check if in IRQ Mode
/// \return     true=IRQ, false=thread
__STATIC_INLINE bool_t IsIrqMode (void) {
  return (__get_IPSR() != 0U);
}

/// Check if IRQ is Masked
/// \return     true=masked, false=not masked
__STATIC_INLINE bool_t IsIrqMasked (void) {
#if   ((defined(__ARM_ARCH_7M__)        && (__ARM_ARCH_7M__        != 0)) || \
       (defined(__ARM_ARCH_7EM__)       && (__ARM_ARCH_7EM__       != 0)) || \
       (defined(__ARM_ARCH_8M_MAIN__)   && (__ARM_ARCH_8M_MAIN__   != 0)) || \
       (defined(__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ != 0)))
  return ((__get_PRIMASK() != 0U) || (__get_BASEPRI() != 0U));
#else
  return  (__get_PRIMASK() != 0U);
#endif
}


//  ==== Core Peripherals functions ====

/// Setup SVC and PendSV System Service Calls
__STATIC_INLINE void SVC_Setup (void) {
#if   ((defined(__ARM_ARCH_8M_MAIN__)   && (__ARM_ARCH_8M_MAIN__   != 0)) || \
       (defined(__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ != 0)) || \
       (defined(__CORTEX_M)             && (__CORTEX_M == 7U)))
  uint32_t p, n;

  SCB->SHPR[10] = 0xFFU;
  n = 32U - (uint32_t)__CLZ(~(SCB->SHPR[10] | 0xFFFFFF00U));
  p = NVIC_GetPriorityGrouping();
  if (p >= n) {
    n = p + 1U;
  }
  SCB->SHPR[7] = (uint8_t)(0xFEU << n);
#elif  (defined(__ARM_ARCH_8M_BASE__)   && (__ARM_ARCH_8M_BASE__   != 0))
  uint32_t n;

  SCB->SHPR[1] |= 0x00FF0000U;
  n = SCB->SHPR[1];
  SCB->SHPR[0] |= (n << (8+1)) & 0xFC000000U;
#elif ((defined(__ARM_ARCH_7M__)        && (__ARM_ARCH_7M__        != 0)) || \
       (defined(__ARM_ARCH_7EM__)       && (__ARM_ARCH_7EM__       != 0)))
  uint32_t p, n;

  SCB->SHP[10] = 0xFFU;
  n = 32U - (uint32_t)__CLZ(~(SCB->SHP[10] | 0xFFFFFF00U));
  p = NVIC_GetPriorityGrouping();
  if (p >= n) {
    n = p + 1U;
  }
  SCB->SHP[7] = (uint8_t)(0xFEU << n);
#elif  (defined(__ARM_ARCH_6M__)        && (__ARM_ARCH_6M__        != 0))
  uint32_t n;

  SCB->SHP[1] |= 0x00FF0000U;
  n = SCB->SHP[1];
  SCB->SHP[0] |= (n << (8+1)) & 0xFC000000U;
#endif
}

/// Get Pending SV (Service Call) Flag
/// \return     Pending SV Flag
__STATIC_INLINE uint8_t GetPendSV (void) {
  return ((uint8_t)((SCB->ICSR & (SCB_ICSR_PENDSVSET_Msk)) >> 24));
}

/// Clear Pending SV (Service Call) Flag
__STATIC_INLINE void ClrPendSV (void) {
  SCB->ICSR = SCB_ICSR_PENDSVCLR_Msk;
}

/// Set Pending SV (Service Call) Flag
__STATIC_INLINE void SetPendSV (void) {
  SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}


//  ==== Service Calls definitions ====

//lint -save -e9023 -e9024 -e9026 "Function-like macros using '#/##'" [MISRA Note 10]

#if defined(__CC_ARM)

#if   ((defined(__ARM_ARCH_7M__)        && (__ARM_ARCH_7M__        != 0)) ||   \
       (defined(__ARM_ARCH_7EM__)       && (__ARM_ARCH_7EM__       != 0)) ||   \
       (defined(__ARM_ARCH_8M_MAIN__)   && (__ARM_ARCH_8M_MAIN__   != 0)) ||   \
       (defined(__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ != 0)))
#define __SVC_INDIRECT(n) __svc_indirect(n)
#elif ((defined(__ARM_ARCH_6M__)        && (__ARM_ARCH_6M__        != 0)) ||   \
       (defined(__ARM_ARCH_8M_BASE__)   && (__ARM_ARCH_8M_BASE__   != 0)))
#define __SVC_INDIRECT(n) __svc_indirect_r7(n)
#endif

#define SVC0_0N(f,t)                                                           \
__SVC_INDIRECT(0) t    svc##f (t(*)());                                        \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (void) {                                         \
  svc##f(svcRtx##f);                                                           \
}

#define SVC0_0(f,t)                                                            \
__SVC_INDIRECT(0) t    svc##f (t(*)());                                        \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (void) {                                         \
  return svc##f(svcRtx##f);                                                    \
}

#define SVC0_1N(f,t,t1)                                                        \
__SVC_INDIRECT(0) t    svc##f (t(*)(t1),t1);                                   \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (t1 a1) {                                        \
  svc##f(svcRtx##f,a1);                                                        \
}

#define SVC0_1(f,t,t1)                                                         \
__SVC_INDIRECT(0) t    svc##f (t(*)(t1),t1);                                   \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (t1 a1) {                                        \
  return svc##f(svcRtx##f,a1);                                                 \
}

#define SVC0_2(f,t,t1,t2)                                                      \
__SVC_INDIRECT(0) t    svc##f (t(*)(t1,t2),t1,t2);                             \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (t1 a1, t2 a2) {                                 \
  return svc##f(svcRtx##f,a1,a2);                                              \
}

#define SVC0_3(f,t,t1,t2,t3)                                                   \
__SVC_INDIRECT(0) t    svc##f (t(*)(t1,t2,t3),t1,t2,t3);                       \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (t1 a1, t2 a2, t3 a3) {                          \
  return svc##f(svcRtx##f,a1,a2,a3);                                           \
}

#define SVC0_4(f,t,t1,t2,t3,t4)                                                \
__SVC_INDIRECT(0) t    svc##f (t(*)(t1,t2,t3,t4),t1,t2,t3,t4);                 \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (t1 a1, t2 a2, t3 a3, t4 a4) {                   \
  return svc##f(svcRtx##f,a1,a2,a3,a4);                                        \
}

#elif defined(__ICCARM__)

#if   ((defined(__ARM_ARCH_7M__)        && (__ARM_ARCH_7M__        != 0)) ||   \
       (defined(__ARM_ARCH_7EM__)       && (__ARM_ARCH_7EM__       != 0)) ||   \
       (defined(__ARM_ARCH_8M_MAIN__)   && (__ARM_ARCH_8M_MAIN__   != 0)) ||   \
       (defined(__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ != 0)))
#define SVC_ArgF(f)                                                            \
  __asm(                                                                       \
    "mov r12,%0\n"                                                             \
    :: "r"(&f): "r12"                                                          \
  );
#elif ((defined(__ARM_ARCH_6M__)        && (__ARM_ARCH_6M__        != 0)) ||   \
       (defined(__ARM_ARCH_8M_BASE__)   && (__ARM_ARCH_8M_BASE__   != 0)))
#define SVC_ArgF(f)                                                            \
  __asm(                                                                       \
    "mov r7,%0\n"                                                              \
    :: "r"(&f): "r7"                                                           \
  );
#endif

#define STRINGIFY(a) #a
#define __SVC_INDIRECT(n) _Pragma(STRINGIFY(swi_number = n)) __swi

#define SVC0_0N(f,t)                                                           \
__SVC_INDIRECT(0) t    svc##f ();                                              \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (void) {                                         \
  SVC_ArgF(svcRtx##f);                                                         \
  svc##f();                                                                    \
}

#define SVC0_0(f,t)                                                            \
__SVC_INDIRECT(0) t    svc##f ();                                              \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (void) {                                         \
  SVC_ArgF(svcRtx##f);                                                         \
  return svc##f();                                                             \
}

#define SVC0_1N(f,t,t1)                                                        \
__SVC_INDIRECT(0) t    svc##f (t1 a1);                                         \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (t1 a1) {                                        \
  SVC_ArgF(svcRtx##f);                                                         \
  svc##f(a1);                                                                  \
}

#define SVC0_1(f,t,t1)                                                         \
__SVC_INDIRECT(0) t    svc##f (t1 a1);                                         \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (t1 a1) {                                        \
  SVC_ArgF(svcRtx##f);                                                         \
  return svc##f(a1);                                                           \
}

#define SVC0_2(f,t,t1,t2)                                                      \
__SVC_INDIRECT(0) t    svc##f (t1 a1, t2 a2);                                  \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (t1 a1, t2 a2) {                                 \
  SVC_ArgF(svcRtx##f);                                                         \
  return svc##f(a1,a2);                                                        \
}

#define SVC0_3(f,t,t1,t2,t3)                                                   \
__SVC_INDIRECT(0) t    svc##f (t1 a1, t2 a2, t3 a3);                           \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (t1 a1, t2 a2, t3 a3) {                          \
  SVC_ArgF(svcRtx##f);                                                         \
  return svc##f(a1,a2,a3);                                                     \
}

#define SVC0_4(f,t,t1,t2,t3,t4)                                                \
__SVC_INDIRECT(0) t    svc##f (t1 a1, t2 a2, t3 a3, t4 a4);                    \
__attribute__((always_inline))                                                 \
__STATIC_INLINE   t  __svc##f (t1 a1, t2 a2, t3 a3, t4 a4) {                   \
  SVC_ArgF(svcRtx##f);                                                         \
  return svc##f(a1,a2,a3,a4);                                                  \
}

#else   // !(defined(__CC_ARM) || defined(__ICCARM__))

//lint -esym(522,__svc*) "Functions '__svc*' are impure (side-effects)"

#if   ((defined(__ARM_ARCH_7M__)        && (__ARM_ARCH_7M__        != 0)) ||   \
       (defined(__ARM_ARCH_7EM__)       && (__ARM_ARCH_7EM__       != 0)) ||   \
       (defined(__ARM_ARCH_8M_MAIN__)   && (__ARM_ARCH_8M_MAIN__   != 0)) ||   \
       (defined(__ARM_ARCH_8_1M_MAIN__) && (__ARM_ARCH_8_1M_MAIN__ != 0)))
#define SVC_RegF "r12"
#elif ((defined(__ARM_ARCH_6M__)        && (__ARM_ARCH_6M__        != 0)) ||   \
       (defined(__ARM_ARCH_8M_BASE__)   && (__ARM_ARCH_8M_BASE__   != 0)))
#define SVC_RegF "r7"
#endif

#define SVC_ArgN(n) \
register uint32_t __r##n __ASM("r"#n)

#define SVC_ArgR(n,a) \
register uint32_t __r##n __ASM("r"#n) = (uint32_t)a

#define SVC_ArgF(f) \
register uint32_t __rf   __ASM(SVC_RegF) = (uint32_t)f

#define SVC_In0 "r"(__rf)
#define SVC_In1 "r"(__rf),"r"(__r0)
#define SVC_In2 "r"(__rf),"r"(__r0),"r"(__r1)
#define SVC_In3 "r"(__rf),"r"(__r0),"r"(__r1),"r"(__r2)
#define SVC_In4 "r"(__rf),"r"(__r0),"r"(__r1),"r"(__r2),"r"(__r3)

#define SVC_Out0
#define SVC_Out1 "=r"(__r0)

#define SVC_CL0
#define SVC_CL1 "r1"
#define SVC_CL2 "r0","r1"

#define SVC_Call0(in, out, cl)                                                 \
  __ASM volatile ("svc 0" : out : in : cl)

#define SVC0_0N(f,t)                                                           \
__attribute__((always_inline))                                                 \
__STATIC_INLINE t __svc##f (void) {                                            \
  SVC_ArgF(svcRtx##f);                                                         \
  SVC_Call0(SVC_In0, SVC_Out0, SVC_CL2);                                       \
}

#define SVC0_0(f,t)                                                            \
__attribute__((always_inline))                                                 \
__STATIC_INLINE t __svc##f (void) {                                            \
  SVC_ArgN(0);                                                                 \
  SVC_ArgF(svcRtx##f);                                                         \
  SVC_Call0(SVC_In0, SVC_Out1, SVC_CL1);                                       \
  return (t) __r0;                                                             \
}

#define SVC0_1N(f,t,t1)                                                        \
__attribute__((always_inline))                                                 \
__STATIC_INLINE t __svc##f (t1 a1) {                                           \
  SVC_ArgR(0,a1);                                                              \
  SVC_ArgF(svcRtx##f);                                                         \
  SVC_Call0(SVC_In1, SVC_Out0, SVC_CL1);                                       \
}

#define SVC0_1(f,t,t1)                                                         \
__attribute__((always_inline))                                                 \
__STATIC_INLINE t __svc##f (t1 a1) {                                           \
  SVC_ArgR(0,a1);                                                              \
  SVC_ArgF(svcRtx##f);                                                         \
  SVC_Call0(SVC_In1, SVC_Out1, SVC_CL1);                                       \
  return (t) __r0;                                                             \
}

#define SVC0_2(f,t,t1,t2)                                                      \
__attribute__((always_inline))                                                 \
__STATIC_INLINE t __svc##f (t1 a1, t2 a2) {                                    \
  SVC_ArgR(0,a1);                                                              \
  SVC_ArgR(1,a2);                                                              \
  SVC_ArgF(svcRtx##f);                                                         \
  SVC_Call0(SVC_In2, SVC_Out1, SVC_CL0);                                       \
  return (t) __r0;                                                             \
}

#define SVC0_3(f,t,t1,t2,t3)                                                   \
__attribute__((always_inline))                                                 \
__STATIC_INLINE t __svc##f (t1 a1, t2 a2, t3 a3) {                             \
  SVC_ArgR(0,a1);                                                              \
  SVC_ArgR(1,a2);                                                              \
  SVC_ArgR(2,a3);                                                              \
  SVC_ArgF(svcRtx##f);                                                         \
  SVC_Call0(SVC_In3, SVC_Out1, SVC_CL0);                                       \
  return (t) __r0;                                                             \
}

#define SVC0_4(f,t,t1,t2,t3,t4)                                                \
__attribute__((always_inline))                                                 \
__STATIC_INLINE t __svc##f (t1 a1, t2 a2, t3 a3, t4 a4) {                      \
  SVC_ArgR(0,a1);                                                              \
  SVC_ArgR(1,a2);                                                              \
  SVC_ArgR(2,a3);                                                              \
  SVC_ArgR(3,a4);                                                              \
  SVC_ArgF(svcRtx##f);                                                         \
  SVC_Call0(SVC_In4, SVC_Out1, SVC_CL0);                                       \
  return (t) __r0;                                                             \
}

#endif

//lint -restore [MISRA Note 10]


//  ==== Exclusive Access Operation ====

#if (EXCLUSIVE_ACCESS == 1)
#include "atomic_cm.h"
#endif  // (EXCLUSIVE_ACCESS == 1)


#endif  // RTX_CORE_CM_H_
