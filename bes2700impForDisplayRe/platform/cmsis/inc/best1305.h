/**************************************************************************//**
 * @file     best1305.h
 * @brief    CMSIS Core Peripheral Access Layer Header File for
 *           ARMCM4 Device Series
 * @version  V2.02
 * @date     10. September 2014
 *
 * @note     configured for CM4 with FPU
 *
 ******************************************************************************/
/* Copyright (c) 2011 - 2014 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/


#ifndef __BEST1305_H__
#define __BEST1305_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ASSEMBLER__
/* -------------------------  Interrupt Number Definition  ------------------------ */

typedef enum IRQn
{
/* -------------------  Cortex-M33 Processor Exceptions Numbers  ------------------ */
    NonMaskableInt_IRQn         = -14,      /*!<  2 Non Maskable Interrupt          */
    HardFault_IRQn              = -13,      /*!<  3 HardFault Interrupt             */
    MemoryManagement_IRQn       = -12,      /*!<  4 Memory Management Interrupt     */
    BusFault_IRQn               = -11,      /*!<  5 Bus Fault Interrupt             */
    UsageFault_IRQn             = -10,      /*!<  6 Usage Fault Interrupt           */
    SVCall_IRQn                 =  -5,      /*!< 11 SV Call Interrupt               */
    DebugMonitor_IRQn           =  -4,      /*!< 12 Debug Monitor Interrupt         */
    PendSV_IRQn                 =  -2,      /*!< 14 Pend SV Interrupt               */
    SysTick_IRQn                =  -1,      /*!< 15 System Tick Interrupt           */

/* --------------------  BEST2300A Specific Interrupt Numbers  -------------------- */
    FPU_IRQn                    =  0,      /*!< FPU Interrupt                      */
    AUDMA_IRQn                  =  1,      /*!< Audio DMA Interrupt                */
    GPDMA_IRQn                  =  2,      /*!< General Purpose DMA Interrupt      */
    AON_WDT_IRQn                =  3,      /*!< AON Watchdog Timer Interrupt       */
    WAKEUP_IRQn                 =  4,      /*!< Wakeup Interrupt                   */
    AON_GPIO_IRQn               =  5,      /*!< AON GPIO Interrupt                 */
    MCU_WDT_IRQn                =  6,      /*!< Watchdog Timer Interrupt           */
    MCU_TIMER00_IRQn            =  7,      /*!< Timer00 Interrupt                  */
    MCU_TIMER01_IRQn            =  8,      /*!< Timer01 Interrupt                  */
    I2C0_IRQn                   =  9,      /*!< I2C0 Interrupt                     */
    SPI0_IRQn                   = 10,      /*!< SPI0 Interrupt                     */
    UART0_IRQn                  = 11,      /*!< UART0 Interrupt                    */
    UART1_IRQn                  = 12,      /*!< UART1 Interrupt                    */
    CODEC_IRQn                  = 13,      /*!< CODEC Interrupt                    */
    BTPCM_IRQn                  = 14,      /*!< BTPCM Interrupt                    */
    I2S0_IRQn                   = 15,      /*!< I2S0 Interrupt                     */
    BT_IRQn                     = 16,      /*!< BT to MCU Interrupt                */
    MCU_TIMER10_IRQn            = 17,      /*!< Timer10 Interrupt                  */
    MCU_TIMER11_IRQn            = 18,      /*!< Timer11 Interrupt                  */
    I2C1_IRQn                   = 19,      /*!< I2C1 Interrupt                     */
    DUMP_IRQn                   = 20,      /*!< DUMP Interrupt                     */
    ISDONE_IRQn                 = 21,      /*!< Intersys MCU2BT Data Done Interrupt */
    ISDONE1_IRQn                = 22,      /*!< Intersys MCU2BT Data1 Done Interrupt */
    ISDATA_IRQn                 = 23,      /*!< Intersys BT2MCU Data Indication Interrupt */
    ISDATA1_IRQn                = 24,      /*!< Intersys BT2MCU Data1 Indication Interrupt */
    CP2MCU_DATA_IRQn            = 25,      /*!< Intersys CP2MCU Data Indication Interrupt */
    CP2MCU_DATA1_IRQn           = 26,      /*!< Intersys CP2MCU Data1 Indication Interrupt */
    CP2MCU_DATA2_IRQn           = 27,      /*!< Intersys CP2MCU Data Indication Interrupt */
    CP2MCU_DATA3_IRQn           = 28,      /*!< Intersys CP2MCU Data1 Indication Interrupt */
    GPADC_IRQn                  = 29,      /*!< GPADC Interrupt                    */
    CHARGER_IRQn                = 30,      /*!< Charger Interrupt                  */
    PMU_IRQn                    = 31,      /*!< PMU Interrupt                      */
    AON_IRQn                    = 32,      /*!< AON Interrupt                      */
    AON_TIMER00_IRQn            = 33,      /*!< AON Timer00 Interrupt              */
    AON_TIMER01_IRQn            = 34,      /*!< AON Timer01 Interrupt              */
    MCU2CP_DONE_IRQn            = 35,      /*!< Intersys MCU2CP Data Done Interrupt */
    MCU2CP_DONE1_IRQn           = 36,      /*!< Intersys MCU2CP Data Done Interrupt */
    MCU2CP_DONE2_IRQn           = 37,      /*!< Intersys MCU2CP Data Done Interrupt */
    MCU2CP_DONE3_IRQn           = 38,      /*!< Intersys MCU2CP Data Done Interrupt */
    BT_STAMP_IRQn               = 39,      /*!< BT Playtime Stamp Interrupt        */
    PWRKEY_IRQn                 = 40,      /*!< Power Key Interrupt                */
    I2C2_IRQn                   = 41,      /*!< I2C2 Interrupt                     */

    USER_IRQn_QTY,
    INVALID_IRQn                = USER_IRQn_QTY,
} IRQn_Type;

#define GPIO_IRQn               AON_GPIO_IRQn
#define TIMER00_IRQn            MCU_TIMER00_IRQn
#define TIMER01_IRQn            MCU_TIMER01_IRQn
#define TIMER10_IRQn            MCU_TIMER10_IRQn
#define TIMER11_IRQn            MCU_TIMER11_IRQn
#define WDT_IRQn                AON_WDT_IRQn
#define RTC_IRQn                PMU_IRQn

#endif

/* ================================================================================ */
/* ================      Processor and Core Peripheral Section     ================ */
/* ================================================================================ */

/* --------  Configuration of Core Peripherals  ----------------------------------- */
#define __CM33_REV                0x0000U   /* Core revision r0p1 */
#define __SAUREGION_PRESENT       0U        /* SAU regions present */
#define __MPU_PRESENT             1U        /* MPU present */
#define __VTOR_PRESENT            1U        /* VTOR present */
#define __NVIC_PRIO_BITS          3U        /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT             1U        /* FPU present */
#define __DSP_PRESENT             1U        /* DSP extension present */

#include "core_cm33.h"                      /* Processor and core peripherals */

#ifndef __ASSEMBLER__

#include "system_ARMCM.h"                  /* System Header                                     */

#endif

/* ================================================================================ */
/* ================       Device Specific Peripheral Section       ================ */
/* ================================================================================ */

/* -------------------  Start of section using anonymous unions  ------------------ */
#if   defined (__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined (__ICCARM__)
  #pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wc11-extensions"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning 586
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif

/* --------------------  End of section using anonymous unions  ------------------- */
#if   defined (__CC_ARM)
  #pragma pop
#elif defined (__ICCARM__)
  /* leave anonymous unions enabled */
#elif (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
  #pragma clang diagnostic pop
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning restore
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif

#ifdef __cplusplus
}
#endif

#endif
