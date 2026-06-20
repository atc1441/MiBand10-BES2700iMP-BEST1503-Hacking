/**************************************************************************//**
 * @file     os_systick.c
 * @brief    CMSIS OS Tick SysTick implementation
 * @version  V1.0.1
 * @date     24. November 2017
 ******************************************************************************/
/*
 * Copyright (c) 2017-2017 ARM Limited. All rights reserved.
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
#include "hal_timer_fast_irq.h"
#include "RTX_Config.h"
#include "os_tick.h"

//lint -emacro((923,9078),SCB,SysTick) "cast from unsigned long to pointer"
#include "RTE_Components.h"
#include CMSIS_device_header

#ifdef  SysTick

#ifndef SYSTICK_IRQ_PRIORITY
#define SYSTICK_IRQ_PRIORITY    0xFFU
#endif

#ifdef CPU_SYSTICK_CLOCK
#define OS_CLOCK            CPU_SYSTICK_CLOCK
#elif defined(OSTICK_USE_FAST_TIMER)
#define OS_CLOCK            CONFIG_FAST_SYSTICK_HZ
#else
#define OS_CLOCK            CONFIG_SYSTICK_HZ
#endif

#if defined  (CHIP_SUBSYS_SENS) && (CHIP_BEST1501P)
#undef OS_CLOCK
#define OS_CLOCK            12000000
#endif

uint32_t SystemCoreClock;
uint32_t Remainder;
uint32_t Load;

#ifndef SYSTICK_CLKSRC_CPU
#define SYSTICK_EXTERNAL_CLOCK          1
#endif

void SystemCoreClockUpdate()
{
    SystemCoreClock = OS_CLOCK;
}
extern void SysTick_Handler(void);
static uint8_t PendST;
// Setup OS Tick.
__WEAK int32_t OS_Tick_Setup (uint32_t freq, IRQHandler_t handler) {
  uint32_t load;
  (void)handler;

  if (freq == 0U) {
    //lint -e{904} "Return statement before end of function"
    return (-1);
  }

  load = (SystemCoreClock / freq) - 1U;
  Remainder = SystemCoreClock % freq;
  Load = load;
  if (load > 0x00FFFFFFU) {
    //lint -e{904} "Return statement before end of function"
    return (-1);
  }

  // Set SysTick Interrupt Priority
#if   ((defined(__ARM_ARCH_8M_MAIN__) && (__ARM_ARCH_8M_MAIN__ != 0)) || \
       (defined(__CORTEX_M)           && (__CORTEX_M           == 7U)))
  SCB->SHPR[11] = SYSTICK_IRQ_PRIORITY;
#elif  (defined(__ARM_ARCH_8M_BASE__) && (__ARM_ARCH_8M_BASE__ != 0))
  SCB->SHPR[1] |= ((uint32_t)SYSTICK_IRQ_PRIORITY << 24);
#elif ((defined(__ARM_ARCH_7M__)      && (__ARM_ARCH_7M__      != 0)) || \
       (defined(__ARM_ARCH_7EM__)     && (__ARM_ARCH_7EM__     != 0)))
  SCB->SHP[11]  = SYSTICK_IRQ_PRIORITY;
#elif  (defined(__ARM_ARCH_6M__)      && (__ARM_ARCH_6M__      != 0))
  SCB->SHP[1]  |= ((uint32_t)SYSTICK_IRQ_PRIORITY << 24);
#else
#error "Unknown ARM Core!"
#endif

#ifdef OSTICK_USE_FAST_TIMER
  hal_fast_timer_setup(HAL_TIMER_TYPE_PERIODIC, (HAL_TIMER_IRQ_HANDLER_T)SysTick_Handler);
  NVIC_SetPriority(OS_Tick_GetIRQn(), (1UL << __NVIC_PRIO_BITS) - 1UL);
  hal_fast_timer_start(US_TO_FAST_TICKS(1000000/OS_TICK_FREQ));
  hal_fast_timer_pause();
#else /*OSTICK_USE_FAST_TIMER*/
#if (SYSTICK_EXTERNAL_CLOCK)
  SysTick->CTRL =  SysTick_CTRL_TICKINT_Msk;
#else
  SysTick->CTRL =  SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk;
#endif
  SysTick->LOAD =  load;
  SysTick->VAL  =  0U;
#endif /*OSTICK_USE_FAST_TIMER*/
  PendST = 0U;

  return (0);
}

/// Enable OS Tick.
__WEAK void OS_Tick_Enable (void) {

  if (PendST != 0U) {
    PendST = 0U;
    SCB->ICSR = SCB_ICSR_PENDSTSET_Msk;
  }

#ifdef OSTICK_USE_FAST_TIMER
  hal_fast_timer_continue();
#else
  SysTick->CTRL |=  SysTick_CTRL_ENABLE_Msk;
#endif
}

/// Disable OS Tick.
__WEAK void OS_Tick_Disable (void) {
#ifdef OSTICK_USE_FAST_TIMER
  hal_fast_timer_pause();
#else
  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
#endif

  if ((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) != 0U) {
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
    PendST = 1U;
  }
}

// Acknowledge OS Tick IRQ.
__WEAK void OS_Tick_AcknowledgeIRQ (void) {
#ifdef OSTICK_USE_FAST_TIMER
#else
  (void)SysTick->CTRL;
#endif
}

// Get OS Tick IRQ number.
__WEAK int32_t  OS_Tick_GetIRQn (void) {
#ifdef OSTICK_USE_FAST_TIMER
  return ((int32_t)TIMER11_IRQn);
#else
  return ((int32_t)SysTick_IRQn);
#endif
}

// Get OS Tick clock.
__WEAK uint32_t OS_Tick_GetClock (void) {
  return (SystemCoreClock);
}

// Get OS Tick interval.
__WEAK uint32_t OS_Tick_GetInterval (void) {
#ifdef OSTICK_USE_FAST_TIMER
  return hal_fast_timer_get_load();
#else
  return (SysTick->LOAD + 1U);
#endif
}

// Get OS Tick count value.
__WEAK uint32_t OS_Tick_GetCount (void) {
#ifdef OSTICK_USE_FAST_TIMER
  return hal_fast_timer_get_count();
#else
  uint32_t load = SysTick->LOAD;
  return  (load - SysTick->VAL);
#endif
}

// Get OS Tick overflow status.
__WEAK uint32_t OS_Tick_GetOverflow (void) {
#ifdef OSTICK_USE_FAST_TIMER
  return 0;
#else
  return ((SysTick->CTRL >> 16) & 1U);
#endif
}

#endif  // SysTick
