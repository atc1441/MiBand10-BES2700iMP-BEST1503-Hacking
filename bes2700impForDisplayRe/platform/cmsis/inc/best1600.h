/**************************************************************************//**
 * @file     best1600.h
 * @brief    CMSIS Core Peripheral Access Layer Header File for
 *           ARMCM55 Device Series (configured for ARMCM55 with double precision FPU,
 *                                  DSP extension, MVE, TrustZone)
 * @version  V1.0.0
 * @date     27. March 2020
 ******************************************************************************/
/*
 * Copyright (c) 2020 Arm Limited. All rights reserved.
   Copyright (c) 2021 BES
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


#ifndef __BEST1600_H__
#define __BEST1600_H__

#ifdef CHIP_SUBSYS_BTH

#include _TO_STRING(CONCAT_NAME(CHIP_ID_LITERAL, _bth.h))

#elif defined(CHIP_SUBSYS_SENS)

#include _TO_STRING(CONCAT_NAME(CHIP_ID_LITERAL, _sens.h))

#else

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ASSEMBLER__
/* -------------------------  Interrupt Number Definition  ------------------------ */

typedef enum IRQn
{
/* -------------------  Processor Exceptions Numbers  ----------------------------- */
    NonMaskableInt_IRQn         = -14,      /*!<  2 Non Maskable Interrupt          */
    HardFault_IRQn              = -13,      /*!<  3 HardFault Interrupt             */
    MemoryManagement_IRQn       = -12,      /*!<  4 Memory Management Interrupt     */
    BusFault_IRQn               = -11,      /*!<  5 Bus Fault Interrupt             */
    UsageFault_IRQn             = -10,      /*!<  6 Usage Fault Interrupt           */
    SecureFault_IRQn            =  -9,      /*!<  7 Secure Fault Interrupt          */
    SVCall_IRQn                 =  -5,      /*!< 11 SV Call Interrupt               */
    DebugMonitor_IRQn           =  -4,      /*!< 12 Debug Monitor Interrupt         */
    PendSV_IRQn                 =  -2,      /*!< 14 Pend SV Interrupt               */
    SysTick_IRQn                =  -1,      /*!< 15 System Tick Interrupt           */

/* -------------------  Processor Interrupt Numbers  ------------------------------ */
    DISPLAY_IRQn                =   0,      /*!< DISPLAY Interrupt                  */
    DSI_IRQn                    =   1,      /*!< DSI Interrupt                      */
    BT_STAMP_IRQn               =   2,      /*!< BT Playtime Stamp Interrupt        */
    BT_IRQn                     =   BT_STAMP_IRQn, /*!< BT All Interrupt            */
    USB_IRQn                    =   3,      /*!< USB Interrupt                      */
    SDMMC0_IRQn                 =   4,      /*!< SDMMC Interrupt                    */
    BTH_IRQn                    =   5,      /*!< BTH All Interrupt                  */
    SEDMA_IRQn                  =   6,      /*!< Security Engine DMA Interrupt      */
    SEC_ENG_IRQn                =   7,      /*!< Security Engine Interrupt          */
    I2C0_IRQn                   =   8,      /*!< I2C0 Interrupt                     */
    SPI0_IRQn                   =   9,      /*!< SPI0 Interrupt                     */
    I2S0_IRQn                   =  10,      /*!< I2S0 Interrupt                     */
    SPDIF0_IRQn                 =  11,      /*!< SPDIF0 Interrupt                   */
    TRNG_IRQn                   =  12,      /*!< TRNG Interrupt                     */
    GPU_IRQn                    =  13,      /*!< TRNG Interrupt                     */
    IMGDMA_IRQn                 =  14,      /*!< Audio DMA Interrupt                */
    PMU1_IRQn                   =  15,      /*!< PMU1 Interrupt                     */
    PMU_IRQn                    =  16,      /*!< PMU Interrupt                      */
    USB_PIN_IRQn                =  17,      /*!< USB Pin Interrupt                  */
    AON_GPIO_IRQn               =  18,      /*!< AON GPIO Interrupt                 */
    EMMC_IRQn                   =  19,      /*!< EMMC Interrupt                     */
    WAKEUP_IRQn                 =  20,      /*!< Wakeup Interrupt                   */
    SYS_WDT_IRQn                =  21,      /*!< Watchdog Timer Interrupt           */
    AON_IRQn                    =  22,      /*!< AON Interrupt                      */
    UART0_IRQn                  =  23,      /*!< UART0 Interrupt                    */
    SYS_TIMER00_IRQn            =  24,      /*!< Timer00 Interrupt                  */
    SYS_TIMER01_IRQn            =  25,      /*!< Timer01 Interrupt                  */
    SYS_TIMER10_IRQn            =  26,      /*!< Timer10 Interrupt                  */
    SYS_TIMER11_IRQn            =  27,      /*!< Timer11 Interrupt                  */
    SYS_TIMER20_IRQn            =  28,      /*!< Timer20 Interrupt                  */
    SYS_TIMER21_IRQn            =  29,      /*!< Timer21 Interrupt                  */
    AON_TIMER00_IRQn            =  30,      /*!< AON Timer00 Interrupt              */
    AON_TIMER01_IRQn            =  31,      /*!< AON Timer01 Interrupt              */
    UART1_IRQn                  =  32,      /*!< UART1 Interrupt                    */
    CODEC_TX_PEAK_IRQn          =  33,      /*!< CODEC TX PEAK Interrupt            */
    CODEC_IRQn                  =  34,      /*!< CODEC Interrupt                    */
    AUDMA_IRQn                  =  35,      /*!< Audio DMA Interrupt                */
    GPDMA_IRQn                  =  36,      /*!< General Purpose DMA Interrupt      */
    XDMA_IRQn                   =  37,      /*!< General Purpose DMA Interrupt      */
    SYS2BTH_DONE_IRQn           =  38,      /*!< Intersys SYS2BTH Data Done Interrupt */
    SYS2BTH_DONE1_IRQn          =  39,      /*!< Intersys SYS2BTH Data Done Interrupt */
    BTH2SYS_DATA_IRQn           =  40,      /*!< Intersys BTH2SYS Data Indication Interrupt */
    BTH2SYS_DATA1_IRQn          =  41,      /*!< Intersys BTH2SYS Data1 Indication Interrupt */
    DSP2MCU_DATA_IRQn           =  42,      /*!< Intersys DSP2MCU Data Indication Interrupt */
    DSP2MCU_DATA1_IRQn          =  43,      /*!< Intersys DSP2MCU Data1 Indication Interrupt */
    DSP2MCU_DATA2_IRQn          =  44,      /*!< Intersys DSP2MCU Data2 Indication Interrupt */
    DSP2MCU_DATA3_IRQn          =  45,      /*!< Intersys DSP2MCU Data3 Indication Interrupt */
    MCU2DSP_DONE_IRQn           =  46,      /*!< Intersys MCU2DSP Data Done Interrupt */
    MCU2DSP_DONE1_IRQn          =  47,      /*!< Intersys MCU2DSP Data1 Done Interrupt */
    MCU2DSP_DONE2_IRQn          =  48,      /*!< Intersys MCU2DSP Data2 Done Interrupt */
    MCU2DSP_DONE3_IRQn          =  49,      /*!< Intersys MCU2DSP Data3 Done Interrupt */
    SENS_IRQn                   =  50,      /*!< SENSOR Hub All Interrupt           */
    SENS2SYS_DATA_IRQn          =  51,      /*!< Intersys SENS2SYS Data Indication Interrupt */
    SENS2SYS_DATA1_IRQn         =  52,      /*!< Intersys SENS2SYS Data1 Indication Interrupt */
    SENS2SYS_DATA2_IRQn         =  53,      /*!< Intersys SENS2SYS Data2 Indication Interrupt */
    SENS2SYS_DATA3_IRQn         =  54,      /*!< Intersys SENS2SYS Data3 Indication Interrupt */
    SYS2SENS_DONE_IRQn          =  55,      /*!< Intersys SYS2SENS Data Done Interrupt */
    SYS2SENS_DONE1_IRQn         =  56,      /*!< Intersys SYS2SENS Data1 Done Interrupt */
    SYS2SENS_DONE2_IRQn         =  57,      /*!< Intersys SYS2SENS Data2 Done Interrupt */
    SYS2SENS_DONE3_IRQn         =  58,      /*!< Intersys SYS2SENS Data3 Done Interrupt */
    I2S1_IRQn                   =  59,      /*!< I2S1 Interrupt                     */
    SPILCD_IRQn                 =  60,      /*!< SPILCD Interrupt                   */
    I2C1_IRQn                   =  61,      /*!< I2C0 Interrupt                     */
    SYS_GPIO_IRQn               =  62,      /*!< SYS GPIO Interrupt                 */
    AON_WDT_IRQn                =  63,      /*!< AON Watchdog Timer Interrupt       */
    ISDONE_IRQn                 =  64,      /*!< Intersys SYS2BT Data Done Interrupt */
    ISDONE1_IRQn                =  65,      /*!< Intersys SYS2BT Data1 Done Interrupt */
    ISDATA_IRQn                 =  66,      /*!< Intersys BT2SYS Data Indication Interrupt */
    ISDATA1_IRQn                =  67,      /*!< Intersys BT2SYS Data1 Indication Interrupt */
    SPI_WAKEUP_IRQn             =  68,      /*!< SPI Slave Wakeup Interrupt         */
    AON_TIMER10_IRQn            =  69,      /*!< AON Timer10 Interrupt              */
    AON_TIMER11_IRQn            =  AON_TIMER10_IRQn, /*!< AON Timer11 Interrupt              */
    SYS_TIMER30_IRQn            =  70,      /*!< Timer30 Interrupt                  */
    SYS_TIMER31_IRQn            =  SYS_TIMER30_IRQn, /*!< Timer31 Interrupt                  */
    BTPCM_IRQn                  =  71,      /*!< BTPCM Interrupt                    */

    USER_IRQn_QTY,
    INVALID_IRQn                = USER_IRQn_QTY,
} IRQn_Type;

#define SENS2MCU_DATA_IRQn      SENS2SYS_DATA_IRQn
#define SENS2MCU_DATA1_IRQn     SENS2SYS_DATA1_IRQn
#define SENS2MCU_DATA2_IRQn     SENS2SYS_DATA2_IRQn
#define SENS2MCU_DATA3_IRQn     SENS2SYS_DATA3_IRQn
#define MCU2SENS_DONE_IRQn      SYS2SENS_DONE_IRQn
#define MCU2SENS_DONE1_IRQn     SYS2SENS_DONE1_IRQn
#define MCU2SENS_DONE2_IRQn     SYS2SENS_DONE2_IRQn
#define MCU2SENS_DONE3_IRQn     SYS2SENS_DONE3_IRQn

#define TIMER00_IRQn            AON_TIMER00_IRQn
#define TIMER01_IRQn            AON_TIMER01_IRQn
#define TIMER10_IRQn            SYS_TIMER00_IRQn
#define TIMER11_IRQn            SYS_TIMER01_IRQn
#define TIMER20_IRQn            SYS_TIMER10_IRQn
#define TIMER21_IRQn            SYS_TIMER11_IRQn

#define GPIO_IRQn               AON_IRQn
#define WDT_IRQn                AON_WDT_IRQn

#endif

/* ================================================================================ */
/* ================      Processor and Core Peripheral Section     ================ */
/* ================================================================================ */

/* -------  Start of section using anonymous unions and disabling warnings  ------- */
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


/* --------  Configuration of Core Peripherals  ----------------------------------- */
#define __CM55_REV                0x0001U   /* Core revision r0p1 */
#define __NVIC_PRIO_BITS          3U        /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */
#define __VTOR_PRESENT            1U        /* VTOR present */
#define __MPU_PRESENT             1U        /* MPU present */
#define __MPU_REGION_NUM          16U       /* MPU region number */
#define __FPU_PRESENT             1U        /* FPU present */
#define __FPU_DP                  1U        /* double precision FPU */
#define __DSP_PRESENT             1U        /* DSP extension present */
#define __SAUREGION_PRESENT       1U        /* SAU regions present */
#define __PMU_PRESENT             1U        /* PMU present */
#define __PMU_NUM_EVENTCNT        8U        /* PMU Event Counters */
#define __ICACHE_PRESENT          1U        /* Instruction Cache present */
#define __DCACHE_PRESENT          1U        /* Data Cache present */

#ifdef ARM_CMNS
#if defined(RTOS) && defined(KERNEL_RTX5)
#define RTE_CMSIS_RTOS2_RTX5_ARMV8M_NS
#endif
#endif

#include "core_cm55.h"                      /* Processor and core peripherals */

#ifndef __ASSEMBLER__
#include "system_ARMCM.h"                   /* System Header */
#endif


/* --------  End of section using anonymous unions and disabling warnings  -------- */
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

#endif
