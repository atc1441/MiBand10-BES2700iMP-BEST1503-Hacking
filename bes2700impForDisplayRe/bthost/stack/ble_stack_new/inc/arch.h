/**
 ****************************************************************************************
 *
 * @file arch.h
 *
 * @brief This file contains the definitions of the macros and functions that are
 * architecture dependent.  The implementation of those is implemented in the
 * appropriate architecture directory.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */


#ifndef _ARCH_H_
#define _ARCH_H_

#include "cmsis_os.h"
#include "hal_trace.h"

/**
 ****************************************************************************************
 * @defgroup BLUEGRIP
 * @brief BlueGRiP IP Platform
 *
 * This module contains reference platform components - RICOW.
 *
 *
 * @{
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DRIVERS
 * @ingroup BLUEGRIP
 * @brief Reference IP Platform Drivers
 *
 * This module contains the necessary drivers to run the platform with the
 * RW BT SW protocol stack.
 *
 * This has the declaration of the platform architecture API.
 *
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
// required to define GLOBAL_INT_** macros as inline assembly

#define ECC_MULT_ALGO_TYPE (32)

/*
 * CPU WORD SIZE
 ****************************************************************************************
 */
/// APS3 is a 32-bit CPU
#define CPU_WORD_SIZE   4

/*
 * CPU Endianness
 ****************************************************************************************
 */
/// APS3 is little endian
#define CPU_LE          1

/*
 * DEBUG configuration
 ****************************************************************************************
 */
#if defined(CFG_DBG)
#define PLF_DEBUG          1
#else //CFG_DBG
#define PLF_DEBUG          0
#endif //CFG_DBG

#ifdef CFG_ROM
#define DBG_SWDIAG(bank , field , value)
#endif //CFG_ROM

#define DBG_DATA_TRACE(data, size)

#define DBG_DATA_ALLOC(data)

#define DBG_DATA_FREE(data)

#define DBG_FUNC_ENTER(func)

#define DBG_FUNC_EXIT(func)

#define DBG_MEM_GRANT_CTRL(mem_ptr, enable)

#define DBG_MEM_PERM_SET(mem_ptr, size, write_en, read_en, init_clr)

#define DBG_MEM_INIT(mem_ptr, size)

#define DBG_SWDIAG(bank , field , value)


/*
 * GAIA
 ****************************************************************************************
 */

/// GAIA board
#if defined(CFG_GAIA)
#define PLF_GAIA          1
#else // CFG_GAIA
#define PLF_GAIA          0
#endif // CFG_GAIA

/*
 * Bubble
 ****************************************************************************************
 */

/// Bubble board
#if defined(CFG_BUBBLE)
#define PLF_BUBBLE          1
#else // CFG_GAIA
#define PLF_BUBBLE          0
#endif // CFG_GAIA

/*
 * Display
 ****************************************************************************************
 */

/// Display controller enable/disable
#if defined(CFG_DISPLAY)
#define PLF_DISPLAY          1
#else // CFG_DISPLAY
#define PLF_DISPLAY          0
#endif // CFG_DISPLAY

/*
* LCD
****************************************************************************************
*/

#define PLF_LCD            1//((PLF_DISPLAY) || (PLF_BUBBLE))

/*
* MAILBOX
****************************************************************************************
*/

#define PLF_MBOX            (PLF_GAIA)


/*
* Bi-Directional RAM for Mailbox I/F
****************************************************************************************
*/
#if (PLF_GAIA)
/// BRAM location in address map
#define BRAM_BASE_ADDRESS    (0x50000000)
/// GAIA LEDS location in address map
#define GAIA_LEDS_BASE_ADDRESS (0x48000000)
/// GAIA PB location in address map
#define GAIA_PB_BASE_ADDRESS (GAIA_LEDS_BASE_ADDRESS + 0x20)
#endif //(PLF_GAIA)

/*
 * NVDS
 ****************************************************************************************
 */

/// NVDS
#ifdef CFG_NVDS
#define PLF_NVDS             1
#else // CFG_NVDS
#define PLF_NVDS             0
#endif // CFG_NVDS

/*
 * UART
 ****************************************************************************************
 */

/// UART
#define PLF_UART             1

/*
 * SPI
 ****************************************************************************************
 */

/// SPI
#define PLF_SPI              1

/*
 * PS2
 ****************************************************************************************
 */

/// PS2
#ifdef CFG_PS2
#define PLF_PS2             1
#else // CFG_PS2
#define PLF_PS2             0
#endif // CFG_PS2

/*
 * Accelerometer
 ****************************************************************************************
 */

/// Accelerometer
#define PLF_ACC              0

/*
 * RTC
 ****************************************************************************************
 */

/// RTC
#ifdef CFG_RTC
#define PLF_RTC             1
#else // CFG_RTC
#define PLF_RTC             0
#endif // CFG_RTC

/*
 * LE AUDIO PATH
 ****************************************************************************************
 */
/// AUDIO path enabled
#ifdef CFG_PCM
#define PLF_PCM   1
#else // CFG_PCM
#define PLF_PCM   0
#endif // CFG_PCM

/*
 * Joystick
 ****************************************************************************************
 */

/// Display controller enable/disable
#define PLF_JOYSTICK         (PLF_BUBBLE)

/*
 * NVDS mapping in flash
 ****************************************************************************************
 */

/// NVDS location in FLASH buffer 1 : 0x00F00000 (15 MB)
#define NVDS_FLASH_ADDRESS_1       (0x00F00000)

/// NVDS location in FLASH buffer 2 : 0x00FC0000 (15.75 MB)
#define NVDS_FLASH_ADDRESS_2       (0x00FC0000)

/// NVDS size in FLASH : 0x00000800 (2K)
#define NVDS_FLASH_SIZE            (0x00000800)

/*
 * DEFINES
 ****************************************************************************************
 */

/// Possible errors detected by FW
#define    RESET_NO_ERROR         0x00000000
#define    RESET_MEM_ALLOC_FAIL   0xF2F2F2F2

/// Reset platform and stay in ROM
#define    RESET_TO_ROM           0xA5A5A5A5
/// Reset platform and reload FW
#define    RESET_AND_LOAD_FW      0xC3C3C3C3

/// Exchange memory size limit
#if defined(BT_DUAL_MODE) && BT_DUAL_MODE
#define    EM_SIZE_LIMIT          0x10000
#else
#define    EM_SIZE_LIMIT          0x8000
#endif

/*
 * EXPORTED FUNCTION DECLARATION
 ****************************************************************************************
 */

#if 1//(RW_DEBUG_STACK_PROF)
/**
****************************************************************************************
* @brief Initialise stack memory area.
*
* This function initialises the stack memory with pattern for use in stack profiling.
****************************************************************************************
*/
void stack_init(void);

/**
 ****************************************************************************************
 * @brief Compute size of SW stack used.
 *
 * This function is compute the maximum size stack used by SW.
 *
 * @return Size of stack used (in bytes)
 ****************************************************************************************
 */
uint16_t get_stack_usage(void);
#endif //(RW_DEBUG_STACK_PROF)

/**
 ****************************************************************************************
 * @brief Re-boot FW.
 *
 * This function is used to re-boot the FW when error has been detected, it is the end of
 * the current FW execution.
 * After waiting transfers on UART to be finished, and storing the information that
 * FW has re-booted by itself in a non-loaded area, the FW restart by branching at FW
 * entry point.
 *
 * Note: when calling this function, the code after it will not be executed.
 *
 * @param[in] error      Error detected by FW
 ****************************************************************************************
 */
void platform_reset(uint32_t error);

#if (PLF_DEBUG)
/**
 ****************************************************************************************
 * @brief Print the assertion error reason and loop forever.
 *
 * @param condition C string containing the condition.
 * @param file C string containing file where the assertion is located.
 * @param line Line number in the file where the assertion is located.
 ****************************************************************************************
 */
void assert_err(const char *condition, const char *file, int line);

/**
 ****************************************************************************************
 * @brief Print the assertion error reason and loop forever.
 * The parameter value that is causing the assertion will also be disclosed.
 *
 * @param param0 parameter value 0.
 * @param param1 parameter value 1.
 * @param file C string containing file where the assertion is located.
 * @param line Line number in the file where the assertion is located.
 ****************************************************************************************
 */
void assert_param(int param0, int param1, const char *file, int line);

/**
 ****************************************************************************************
 * @brief Print the assertion warning reason.
 *
 * @param param0 parameter value 0.
 * @param param1 parameter value 1.
 * @param file C string containing file where the assertion is located.
 * @param line Line number in the file where the assertion is located.
 ****************************************************************************************
 */
void assert_warn(int param0, int param1, const char *file, int line);
#endif //PLF_DEBUG


/*
 * ASSERTION CHECK
 ****************************************************************************************
 */
#if 1
/// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR(cond)                             { if (!(cond)) { TRACE(2,"line is %d,function %s", __LINE__, __FUNCTION__); } }

/// Assertions showing a critical error that could require a full system reset
#define ASSERT_INFO(cond, param0, param1)            { if (!(cond)) { TRACE(4,"line is %d,function %s, 0x%x, 0x%x", __LINE__, __FUNCTION__, param0, param1); } }

/// Assertions showing a non-critical problem that has to be fixed by the SW
#define ASSERT_WARN(cond, param0, param1)            { if (!(cond)) { TRACE(4,"line is %d,function %s, 0x%x, 0x%x", __LINE__, __FUNCTION__, param0, param1); } }

#define DUMP_DATA(data, length) \
       // dump_data((uint8_t*)data, length)

#else
/// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR(cond)

/// Assertions showing a critical error that could require a full system reset
#define ASSERT_INFO(cond, param0, param1)

/// Assertions showing a non-critical problem that has to be fixed by the SW
#define ASSERT_WARN(cond, param0, param1)

/// DUMP data array present in the SW.
#define DUMP_DATA(data, length)
#endif

/// Object allocated in shared memory - check linker script
#define __SHARED __attribute__ ((section("shram")))

/// @} DRIVERS
#endif // _ARCH_H_
