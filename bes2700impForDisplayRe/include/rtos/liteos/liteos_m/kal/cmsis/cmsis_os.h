/***************************************************************************
 *
 * Copyright 2021-2031 BES.
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

#ifndef __CMSIS_OS_H__
#define __CMSIS_OS_H__

#ifdef __cplusplus
extern "C"{
#endif

///< @b osCMSIS identifies the CMSIS-RTOS API version.
#if (CMSIS_OS_VER == 2)
    #define osCMSIS 0x20001U    ///< API version (main[31:16].sub[15:0])
    #include "cmsis_os2.h"
#else
    #define osCMSIS 0x10001U    ///< API version (main[31:16].sub[15:0])
    #include <stdint.h>
    #include <stddef.h>
#endif

///< OS
/**@defgroup cmsis_os CMSIS OS Module.
* @{
* @ingroup cmsis
* @brief 标准CMSIS OS接口使用.
*/
#if (osCMSIS < 0x20000U)
typedef enum {
  osPriorityIdle          = -3,         ///< Priority: idle (lowest)
  osPriorityLow           = -2,         ///< Priority: low
  osPriorityBelowNormal   = -1,         ///< Priority: below normal
  osPriorityNormal        =  0,         ///< Priority: normal (default)
  osPriorityAboveNormal   = +1,         ///< Priority: above normal
  osPriorityHigh          = +2,         ///< Priority: high
  osPriorityRealtime      = +3,         ///< Priority: realtime (highest)
  osPriorityError         = 0x84,       ///< System cannot determine priority or illegal priority.
  osPriorityReserved      = 0x7FFFFFFF  ///< Prevents enum down-size compiler optimization.
} osPriority;
#else
typedef osPriority_t osPriority;
#endif

///< Status code values returned by CMSIS-RTOS functions.
#if (osCMSIS < 0x20000U)
typedef enum {
  osOK                    =    0,       ///< Function completed; no error or event occurred.
  osEventSignal           = 0x08,       ///< Function completed; signal event occurred.
  osEventMessage          = 0x10,       ///< Function completed; message event occurred.
  osEventMail             = 0x20,       ///< Function completed; mail event occurred.
  osEventTimeout          = 0x40,       ///< Function completed; timeout occurred.
  osErrorParameter        = 0x80,       ///< Parameter error: a mandatory parameter was missing or specified an incorrect object.
  osErrorResource         = 0x81,       ///< Resource not available: a specified resource was not available.
  osErrorTimeoutResource  = 0xC1,       ///< Resource not available within given time: a specified resource was not available within the timeout period.
  osErrorISR              = 0x82,       ///< Not allowed in ISR context: the function cannot be called from interrupt service routines.
  osErrorISRRecursive     = 0x83,       ///< Function called multiple times from ISR with same object.
  osErrorPriority         = 0x84,       ///< System cannot determine priority or thread has illegal priority.
  osErrorNoMemory         = 0x85,       ///< System is out of memory: it was impossible to allocate or reserve memory for the operation.
  osErrorValue            = 0x86,       ///< Value of a parameter is out of range.
  osErrorOS               = 0xFF,       ///< Unspecified RTOS error: run-time error but no other error message fits.
  osStatusReserved        = 0x7FFFFFFF  ///< Prevents enum down-size compiler optimization.
} osStatus;
#else
typedef osStatus_t              osStatus;
#define osEventSignal           (0x08)
#define osEventMessage          (0x10)
#define osEventMail             (0x20)
#define osEventTimeout          (0x40)
#define osErrorOS               osError
#define osErrorTimeoutResource  osErrorTimeout
#define osErrorISRRecursive     (-126)
#define osErrorValue            (-127)
#define osErrorPriority         (-128)
#define os_status_reserved      osStatusReserved
#endif

// The stack space occupied is mainly dependent on the underling C standard library
#if defined(__GNUC__) || defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__ICCARM__)
#    define WORDS_STACK_SIZE   512
#elif defined(TOOLCHAIN_ARM_MICRO)
#    define WORDS_STACK_SIZE   128
#endif

#define DEFAULT_STACK_SIZE         (WORDS_STACK_SIZE*4)

/// \note CAN BE CHANGED: \b osFeature_xxx identifies RTOS features.
#define osFeature_MainThread  0         ///< main thread      1=main can be thread, 0=not available
#define osFeature_Signals     16U       ///< maximum number of Signal Flags available per thread
#define osFeature_Semaphore   65535U    ///< maximum count for \ref osSemaphoreCreate function
#define osFeature_Wait        0         ///< osWait function: 1=available, 0=not available
#define osFeature_SysTick     1         ///< osKernelSysTick functions: 1=available, 0=not available
#define osFeature_Pool        1         ///< Memory Pools:    1=available, 0=not available
#define osFeature_MessageQ    1         ///< Message Queues:  1=available, 0=not available
#define osFeature_MailQ       1         ///< Mail Queues:     1=available, 0=not available

#define osRtxSemaphoreTokenLimit 65535U ///< maximum number of tokens per semaphore
/**
 * @brief Message ID identifies the message queue.
 * @note CAN BE CHANGED: \b implementation specific in every CMSIS-RTOS.
 */
typedef void *osMessageQId;

/**
 * @brief Mail ID identifies the mail queue.
 * @note CAN BE CHANGED: \b implementation specific in every CMSIS-RTOS.
 */
typedef void* osMailQId;

///< Event structure contains detailed information about an event.
typedef struct {
  osStatus       status;     ///< status code: event or error information
  union {
    uint32_t     v;          ///< message as 32-bit value
    void         *p;         ///< message or mail as void pointer
    int32_t      signals;    ///< signal flags
  } value;                   ///< event value
  union {
    osMailQId    mail_id;    ///< mail id obtained by \ref osMailCreate
    osMessageQId message_id; ///< message id obtained by \ref osMessageCreate
  } def;                     ///< event definition
} osEvent;

/** @} cmsis_os */

/**@defgroup cmsis_kernel CMSIS Kernel Module.
* @{
* @ingroup cmsis
* @brief 标准CMSIS Kernel 接口使用.
*/
#if (osCMSIS < 0x20000U)
/**
 * @brief Initialize the RTOS Kernel for creating objects.
 * @return status code that indicates the execution status of the function.
 */
osStatus osKernelInitialize (void);

/**
 * @brief Start the RTOS Kernel scheduler.
 * @return status code that indicates the execution status of the function.
 */
osStatus osKernelStart (void);

/**
 * @brief Check if the RTOS kernel is already started.
 * @return 0 RTOS is not started, 1 RTOS is started.
 */
int32_t osKernelRunning(void);
#endif

#if (osCMSIS < 0x20000U)

///< The RTOS kernel system timer frequency in Hz.
///< @note Reflects the system timer setting and is typically defined in a configuration file.
#define osKernelSysTickFrequency 100000000

/**
 * @brief Get the RTOS kernel system timer counter.
 * @return RTOS kernel system timer as 32-bit value
 */
uint32_t osKernelSysTick (void);

#define osKernelSysTickFrq() osKernelSysTickFrequency
#else
//#define  osKernelSysTick osKernelGetSysTimerCount
//#define osKernelSysTickFrq() osKernelGetSysTimerFreq()
#define osKernelSysTick osKernelGetTickCount
#define osKernelSysTickFrq() osKernelGetTickFreq()
#endif

/**
 * @brief Convert a microseconds value to a RTOS kernel system timer value.
 * @param         microsec     time value in microseconds.
 * @return time value normalized to the \ref osKernelSysTickFrequency
 */
#define osKernelSysTickMicroSec(microsec) (((uint64_t)microsec * osKernelSysTickFrq()) / 1000000)
/** @} cmsis_kernel */


/**@defgroup cmsis_thread CMSIS thread Module.
* @{
* @ingroup cmsis
* @brief 标准CMSIS thread接口使用.
*/
typedef void (*os_pthread)(void const* args);

#if (osCMSIS < 0x20000U)
typedef void*        osThreadId;
#else
typedef osThreadId_t osThreadId;
#endif

#if (osCMSIS < 0x20000U)
typedef struct os_thread_def {
  os_pthread     pthread;   ///< start address of thread function
  osPriority     tpriority; ///< initial thread priority
  uint32_t       instances; ///< maximum number of instances of that thread function
  uint32_t       stacksize; ///< stack size requirements in bytes; 0 is default stack size
} osThreadDef_t;
#else
typedef struct os_thread_def {
  os_pthread     pthread;   ///< start address of thread function
  osThreadAttr_t attr;      ///< thread attributes
} osThreadDef_t;
#endif

#if (osCMSIS < 0x20000U)
#define osThreadDef(name, priority, instances, stacksz) \
    const osThreadDef_t os_thread_##name = {(name),(priority),(instances),(stacksz)}
#else
/* liteos does not support to use ext stack buf, only need to specify the stack size of thread */
#define osThreadDef(name,priority,instances,stacksz,task_name) \
    const osThreadDef_t os_thread_##name = {(name),{task_name,osThreadDetached,NULL,0U,NULL,8*((stacksz+7)/8),(priority),0U,0U}}
/*  uint64_t os_thread_stack_##name [(8*((stacksz+7)/8))/sizeof(uint64_t)]; \
    const osThreadDef_t os_thread_##name = {(name),{task_name,osThreadDetached,NULL,0U,os_thread_stack_##name,8*((stacksz+7)/8),(priority),0U,0U}}*/
#endif

#define osThread(name) &os_thread_##name

/**
 * @brief Create a thread and add it to Active Threads and set it to state READY.
 * @param[in]     thread_def    thread definition referenced with \ref osThread.
 * @param[in]     argument      pointer that is passed to the thread function as start argument.
 * @return thread ID for reference by other functions or NULL in case of error.
 */
osThreadId osThreadCreate(const osThreadDef_t* thread_def, void* argument);

#if (osCMSIS < 0x20000U)
/**
 * @brief Return the thread ID of the current running thread.
 * @return thread ID for reference by other functions or NULL in case of error.
 */
osThreadId osThreadGetId(void);

/**
 * @brief Change priority of a thread.
 * @param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 * @param[in]     priority      new priority value for the thread function.
 * @return status code that indicates the execution status of the function.
 */
osStatus osThreadSetPriority(osThreadId thread_id, osPriority priority);

/**
 * @brief Get current priority of a thread.
 * @param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 * @return current priority value of the specified thread.
 */
osPriority osThreadGetPriority(osThreadId thread_id);

/**
 * @brief Pass control to next thread that is in state \b READY.
 * @return status code that indicates the execution status of the function.
 */
osStatus osThreadYield (void);

/**
 * @brief Terminate execution of a thread.
 * @param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 * @return status code that indicates the execution status of the function.
 */
osStatus osThreadTerminate(osThreadId thread_id);
#else
const char *osGetThreadName(void);
#endif

int osGetThreadIntId(void);
void os_error_str(const char* fmt, ...);
void osThreadExit(void);

/**
 * @brief Wait for Timeout (Time Delay).
 * @param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue "time delay" value
 * @return status code that indicates the execution status of the function.
 */
#if (osCMSIS < 0x20000U)
osStatus osDelay (uint32_t millisec);
#endif

/**
 * @brief Wait for Signal, Message, Mail, or Timeout.
 * @brief param[in] millisec          \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out
 * @return event that contains signal, message, or mail information or error code.
 */
osEvent osWait (uint32_t millisec);
/** @} cmsis_thread */

/**@defgroup cmsis_signal CMSIS Signal Module.
* @{
* @ingroup cmsis
* @brief 标准CMSIS Signal 接口使用.
*/
/**
 * @brief Set the specified Signal Flags of an active thread.
 * @param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 * @param[in]     signals       specifies the signal flags of the thread that should be set.
 * @return previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters.
 */
int32_t osSignalSet (osThreadId thread_id, int32_t signals);

/**
 * @brief Clear the specified Signal Flags of an active thread.
 * @param[in]     thread_id     thread ID obtained by \ref osThreadCreate or \ref osThreadGetId.
 * @param[in]     signals       specifies the signal flags of the thread that shall be cleared.
 * @return previous signal flags of the specified thread or 0x80000000 in case of incorrect parameters or call from ISR.
 */
int32_t osSignalClear (osThreadId thread_id, int32_t signals);

/**
 * @brief Wait for one or more Signal Flags to become signaled for the current \b RUNNING thread.
 * @param[in]     signals       wait until all specified signal flags set or 0 for any single signal flag.
 * @param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @return event flag information or error code.
 */
osEvent osSignalWait (int32_t signals, uint32_t millisec);
/** @} cmsis_signal */

/**@defgroup cmsis_mutex CMSIS Mutex Module.
* @{
* @ingroup cmsis
* @brief 标准CMSIS Mutex 接口使用.
*/
/**
 * @brief Mutex Definition structure contains setup information for a mutex.
 * @note CAN BE CHANGED: \b os_mutex_def is implementation specific in every CMSIS-RTOS.
 */
#if (osCMSIS < 0x20000U)
typedef struct os_mutex_def {
  uint32_t dummy;   ///< dummy value
}osMutexDef_t;
#else
typedef osMutexAttr_t osMutexDef_t;
#endif

#if (osCMSIS < 0x20000U)
typedef void*       osMutexId;
#else
typedef osMutexId_t osMutexId;
#endif

/**
 * @param         name          name of the mutex object.
 * @note CAN BE CHANGED: The parameter to \b osMutexDef shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#if defined (osObjectsExternal)  // object is external
#define osMutexDef(name) \
extern const osMutexDef_t os_mutex_def_##name
#else                   // define the object
#if (osCMSIS < 0x20000U)
#define osMutexDef(name) \
const osMutexDef_t os_mutex_##name = {0}
#else
#define osMutexDef(name) \
const osMutexDef_t os_mutex_##name = {NULL,osMutexRecursive|osMutexPrioInherit|osMutexRobust,NULL,0U}
#endif
#endif

/************************add osMutexDefEx_t*****************************/
#if (osCMSIS < 0x20000U)
typedef struct os_mutex_def_ex  {
  osMutexDef_t       os_mutex_def;
} osMutexDefEx_t;

#define osMutexInit(pMutex) \
((osMutexDefEx_t*)&pMutex)->os_mutex_def.mutex = 0
#else
typedef struct os_mutex_def_ex  {
  osMutexDef_t       os_mutex_def;
} osMutexDefEx_t;

#define osMutexInit(pMutex) \
((osMutexDefEx_t*)&pMutex)->os_mutex_def.name = NULL; \
((osMutexDefEx_t*)&pMutex)->os_mutex_def.attr_bits = osMutexRecursive | osMutexPrioInherit | osMutexRobust; \
((osMutexDefEx_t*)&pMutex)->os_mutex_def.cb_mem = NULL; \
((osMutexDefEx_t*)&pMutex)->os_mutex_def.cb_size = 0U
#endif
/********************************add osMutexDefEx_t*********************/

/**
 * @brief Access a Mutex definition.
 * @param         name          name of the mutex object.
 * @note CAN BE CHANGED: The parameter to \b osMutex shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#define osMutex(name) &os_mutex_##name

/**
 * @brief Create and Initialize a Mutex object.
 * @param[in]     mutex_def     mutex definition referenced with \ref osMutex.
 * @return mutex ID for reference by other functions or NULL in case of error.
 */
osMutexId osMutexCreate(const osMutexDef_t* mutex_def);


#if (osCMSIS < 0x20000U)
/**
 * @brief Wait until a Mutex becomes available.
 * @param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
 * @param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @return status code that indicates the execution status of the function.
 */
osStatus osMutexWait(osMutexId mutex_id, uint32_t millisec);

/**
 * @brief Release a Mutex that was obtained by \ref osMutexWait.
 * @param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
 * @return status code that indicates the execution status of the function.
 */
osStatus osMutexRelease (osMutexId mutex_id);

/**
 * @brief Delete a Mutex object.
 * @param[in]     mutex_id      mutex ID obtained by \ref osMutexCreate.
 * @return status code that indicates the execution status of the function.
 */
osStatus osMutexDelete (osMutexId mutex_id);
#else
#define  osMutexWait osMutexAcquire
#endif
/** @} cmsis_mutex */


/**@defgroup cmsis_semaphore CMSIS Semaphore Module.
* @{
* @ingroup cmsis
* @brief 标准CMSIS Semaphore 接口使用.
*/
/**
 * @brief Semaphore ID identifies the semaphore.
 * @note CAN BE CHANGED: \b implementation specific in every CMSIS-RTOS.
 */
#if (osCMSIS < 0x20000U)
typedef void*           osSemaphoreId;
#else
typedef osSemaphoreId_t osSemaphoreId;
#endif

/**
 * @brief Semaphore Definition structure contains setup information for a semaphore.
 * @note CAN BE CHANGED: \b os_semaphore_def is implementation specific in every CMSIS-RTOS.
 */
#if (osCMSIS < 0x20000U)
typedef struct os_semaphore_def {
  uint32_t                     dummy;   ///< dummy value
} osSemaphoreDef_t;
#else
typedef osSemaphoreAttr_t osSemaphoreDef_t;
#endif

/**
 * @brief Define a Semaphore object.
 * @param         name          name of the semaphore object.
 * @note CAN BE CHANGED: The parameter to \b osSemaphoreDef shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#if (osCMSIS < 0x20000U)
#define osSemaphoreDef(name) const osSemaphoreDef_t os_semaphore_##name = {0}
#else
#define osSemaphoreDef(name) const osSemaphoreDef_t os_semaphore_##name = { NULL, 0U, NULL, 0U }
#endif

/**
 * @brief Access a Semaphore definition.
 * @param         name          name of the semaphore object.
 * @note CAN BE CHANGED: The parameter to \b osSemaphore shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#define osSemaphore(name) &os_semaphore_##name

/**
 * @briedf Create and Initialize a Semaphore object.
 * @param[in]     semaphore_def semaphore definition referenced with \ref osSemaphore.
 * @param[in]     count         maximum and initial number of available tokens.
 * @return semaphore ID for reference by other functions or NULL in case of error.
 */
osSemaphoreId osSemaphoreCreate(const osSemaphoreDef_t *semaphore_def, int32_t count);

/**
 * @brief Wait until a Semaphore token becomes available.
 * @param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
 * @param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @return number of available tokens, or -1 in case of incorrect parameters.
 */
int32_t osSemaphoreWait (osSemaphoreId semaphore_id, uint32_t millisec);

#if (osCMSIS < 0x20000U)
/**
 * @brief Release a Semaphore token.
 * @param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
 * @return status code that indicates the execution status of the function.
 */
osStatus osSemaphoreRelease (osSemaphoreId semaphore_id);

/**
 * @brief Delete a Semaphore object.
 * @param[in]     semaphore_id  semaphore object referenced with \ref osSemaphoreCreate.
 * @return status code that indicates the execution status of the function.
 */
osStatus osSemaphoreDelete (osSemaphoreId semaphore_id);
#endif
/** @} cmsis_semaphore */

/**@defgroup cmsis_timer CMSIS timer Module.
* @{
* @ingroup cmsis
* @brief 标准CMSIS timer 接口使用.
*/

typedef void (*os_ptimer)(void const* argument);
#if (osCMSIS < 0x20000U)
typedef void*       osTimerId;
#else
typedef osTimerId_t osTimerId;
#endif

#if (osCMSIS < 0x20000U)
typedef enum {
  osTimerOnce     = 0,          ///< One-shot timer.
  osTimerPeriodic = 1           ///< Repeating timer.
} os_timer_type;
#else
typedef osTimerType_t os_timer_type;
#endif

/**
 * @brief Timer Definition structure contains timer parameters.
 * @note CAN BE CHANGED: \b os_timer_def is implementation specific in every CMSIS-RTOS.
 */
#if (osCMSIS < 0x20000U)
typedef struct os_timer_def {
  os_ptimer     ptimer; ///< start address of a timer function
} osTimerDef_t;
#else
typedef struct os_timer_def {
  os_ptimer     ptimer; ///< start address of a timer function
  osTimerAttr_t attr;   ///< timer attributes
} osTimerDef_t;
#endif

/**
 * @brief Define a Timer object.
 * @param         name          name of the timer object.
 * @param         function      name of the timer call back function.
 * @note CAN BE CHANGED: The parameter to \b osTimerDef shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#if defined (osObjectsExternal)  // object is external
#define osTimerDef(name, function) \
extern const osTimerDef_t os_timer_def_##name
#else 
#if (osCMSIS < 0x20000U)
#define osTimerDef(name, function) \
const osTimerDef_t os_timer_##name = { (function) }
#else
#define osTimerDef(name, function) \
const osTimerDef_t os_timer_##name = { (function), { NULL, 0U, NULL, 0U } }
#endif
#endif


/***********************add osTimerDefEx_t**********************/
#if (osCMSIS < 0x20000U)
typedef struct os_timer_def_ex  {
  osTimerDef_t       os_timer_def;
} osTimerDefEx_t;
#define osTimerInit(pTimer,function)  \
((osTimerDefEx_t*)&pTimer)->os_timer_def.ptimer = function

#else
typedef struct os_timer_def_ex  {
  osTimerDef_t       os_timer_def;
} osTimerDefEx_t;

#define osTimerInit(pTimer,function)  \
((osTimerDefEx_t*)&pTimer)->os_timer_def.ptimer = function; \
((osTimerDefEx_t*)&pTimer)->os_timer_def.attr.name = NULL; \
((osTimerDefEx_t*)&pTimer)->os_timer_def.attr.attr_bits = 0U; \
((osTimerDefEx_t*)&pTimer)->os_timer_def.attr.cb_mem = NULL; \
((osTimerDefEx_t*)&pTimer)->os_timer_def.attr.cb_size = 0U
#endif
/***********************add osTimerDefEx_t**********************/

/**
 * @brief Access a Timer definition.
 * @param         name          name of the timer object.
 * @note CAN BE CHANGED: The parameter to \b osTimer shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#define osTimer(name) &os_timer_##name

/**
 * @brief Create and Initialize a timer.
 * @param[in]     timer_def     timer object referenced with \ref osTimer.
 * @param[in]     type          osTimerOnce for one-shot or osTimerPeriodic for periodic behavior.
 * @param[in]     argument      argument to the timer call back function.
 * @return timer ID for reference by other functions or NULL in case of error.
 */
osTimerId osTimerCreate (const osTimerDef_t *timer_def, os_timer_type type, void *argument);

#if (osCMSIS < 0x20000U)
/**
 * @brief Start or restart a timer.
 * @param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
 * @param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue "time delay" value of the timer.
 * @return status code that indicates the execution status of the function.
 */
osStatus osTimerStart (osTimerId timer_id, uint32_t millisec);

/**
 * @brief Stop a timer.
 * @param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
 * @return status code that indicates the execution status of the function.
 */
osStatus osTimerStop (osTimerId timer_id);

/**
 * @brief Delete a timer.
 * @param[in]     timer_id      timer ID obtained by \ref osTimerCreate.
 * @return status code that indicates the execution status of the function.
 */
osStatus osTimerDelete (osTimerId timer_id);
#endif
/** @} cmsis_timer */

///< Message
/**@defgroup cmsis_message CMSIS message Module.
* @{
* @ingroup cmsis
* @brief 标准CMSIS message 接口使用.
*/

#if (osCMSIS < 0x20000U)
typedef struct os_messageQ_def {
  uint32_t             queue_sz; ///< number of elements in the queue
  void                 *pool;    ///< memory array for messages
} osMessageQDef_t;
#else
typedef struct os_messageQ_def {
  uint32_t             queue_sz; ///< number of elements in the queue
  osMessageQueueAttr_t attr;     ///< message queue attributes
} osMessageQDef_t;
#endif

/**
 * @brief Create a Message Queue Definition.
 * @param         name          name of the queue.
 * @param         queue_sz      maximum number of messages in the queue.
 * @param         type          data type of a single message element (for debugger).
 * @note CAN BE CHANGED: The parameter to \b osMessageQDef shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#if (osCMSIS < 0x20000U)
#define osMessageQDef(name, queue_sz, type) const osMessageQDef_t os_msgQ_##name = { (queue_sz), NULL }
#else
#define osMessageQDef(name, queue_sz, type) const osMessageQDef_t os_msgQ_##name = { (queue_sz), { NULL, 0U, NULL, 0U, NULL, 0U } }
#endif

/**
 * @brief Access a Message Queue Definition.
 * @param         name          name of the queue
 * @note CAN BE CHANGED: The parameter to \b osMessageQ shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#define osMessageQ(name) &os_msgQ_##name

/**
 * @brief Create and Initialize a Message Queue object.
 * @param[in]     queue_def     message queue definition referenced with \ref osMessageQ.
 * @param[in]     thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
 * @return message queue ID for reference by other functions or NULL in case of error.
 */
osMessageQId osMessageCreate (const osMessageQDef_t *queue_def, osThreadId thread_id);

/**
 * @brief Put a Message to a Queue.
 * @param[in]     queue_id      message queue ID obtained with \ref osMessageCreate.
 * @param[in]     info          message information.
 * @param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @return status code that indicates the execution status of the function.
 */
osStatus osMessagePut (osMessageQId queue_id, uint32_t info, uint32_t millisec);

/**
 * @brief Get a Message from a Queue or timeout if Queue is empty.
 * @param[in]     queue_id      message queue ID obtained with \ref osMessageCreate.
 * @param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @return event information that includes status code.
 */
osEvent osMessageGet (osMessageQId queue_id, uint32_t millisec);

uint32_t osMessageGetSpace (osMessageQId queue_id);
/** @} cmsis_message */

/**@defgroup cmsis_mempool CMSIS memory pool Module.
* @{
* @ingroup cmsis
* @brief 标准CMSIS memory pool接口使用.
*/
/**
 * @brief Pool ID identifies the memory pool.
 * @note CAN BE CHANGED: \b implementation specific in every CMSIS-RTOS.
 */
typedef void* osPoolId;

/**
 * @brief Definition structure for memory block allocation.
 * @note CAN BE CHANGED: \b os_pool_def is implementation specific in every CMSIS-RTOS.
 */
#if (osCMSIS < 0x20000U)
typedef struct os_pool_def {
  uint32_t            pool_sz; ///< number of items (elements) in the pool
  uint32_t            item_sz; ///< size of an item
  void                *pool;   ///< pointer to memory for pool
} osPoolDef_t;
#else
typedef struct os_pool_def {
  uint32_t           pool_sz;  ///< number of items (elements) in the pool
  uint32_t           item_sz;  ///< size of an item
  osMemoryPoolAttr_t attr;     ///< memory pool attributes
} osPoolDef_t;
#endif

/**
 * @brief Define a Memory Pool.
 * @param         name          name of the memory pool.
 * @param         no            maximum number of blocks (objects) in the memory pool.
 * @param         type          data type of a single block (object).
 * @note CAN BE CHANGED: The parameter to \b osPoolDef shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#if (osCMSIS < 0x20000U)
#define osPoolDef(name, no, type) const osPoolDef_t os_pool_##name = { (no), sizeof(type), NULL }
#else
#define osPoolDef(name, no, type) const osPoolDef_t os_pool_##name = { (no), sizeof(type), { NULL, 0U, NULL, 0U, NULL, 0U } }
#endif

/**
 * @brief Access a Memory Pool definition.
 * @param         name          name of the memory pool
 * @note CAN BE CHANGED: The parameter to \b osPool shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#define osPool(name) &os_pool_##name

/**
 * @brief Create and Initialize a Memory Pool object.
 * @param[in]     pool_def      memory pool definition referenced with \ref osPool.
 * @return memory pool ID for reference by other functions or NULL in case of error.
 */
osPoolId osPoolCreate (const osPoolDef_t *pool_def);

/**
 * @brief Allocate a memory block from a Memory Pool.
 * @param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
 * @return address of the allocated memory block or NULL in case of no memory available.
 */
void *osPoolAlloc (osPoolId pool_id);

/**
 * @brief Allocate a memory block from a Memory Pool and set memory block to zero.
 * @param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
 * @return address of the allocated memory block or NULL in case of no memory available.
 */
void *osPoolCAlloc (osPoolId pool_id);

/**
 * @brief Return an allocated memory block back to a Memory Pool.
 * @param[in]     pool_id       memory pool ID obtain referenced with \ref osPoolCreate.
 * @param[in]     block         address of the allocated memory block to be returned to the memory pool.
 * @return status code that indicates the execution status of the function.
 */
osStatus osPoolFree (osPoolId pool_id, void *block);
/** @} cmsis_mempool */


/**@defgroup cmsis_mailq CMSIS mail queue Module.
* @{
* @ingroup cmsis
* @brief 标准CMSIS mail queue接口使用.
*/

/**
 * @brief Definition structure for mail queue.
 * @note CAN BE CHANGED: \b os_mailQ_def is implementation specific in every CMSIS-RTOS.
 */
#if (osCMSIS < 0x20000U)
typedef struct os_mailQ_def {
  uint32_t               queue_sz; ///< number of elements in the queue
  uint32_t               item_sz;  ///< size of an item
  void                   *pool;    ///< memory array for mail
} osMailQDef_t;
#else
typedef struct os_mailQ_def {
  uint32_t                queue_sz;    ///< number of elements in the queue
  uint32_t                 item_sz;    ///< size of an item
  void                       *pool;    ///< memory array for mail
} osMailQDef_t;
#endif
struct osMailQ {
    uint32_t queue;
    void *pool;
};


/**
 * @brief Create a Mail Queue Definition.
 * @param         name          name of the queue.
 * @param         queue_sz      maximum number of mails in the queue.
 * @param         type          data type of a single mail element.
 * @note CAN BE CHANGED: The parameter to \b osMailQDef shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#if (osCMSIS < 0x20000U)
#define osMailQDef(name, queue_sz, type) const osMailQDef_t os_mailQ_##name = { (queue_sz), sizeof(type), NULL }
#else
#define osMailQDef(name, queue_sz, type) \
    struct osMailQ os_mailQ_p_##name = { 0, NULL }; \
    const osMailQDef_t os_mailQ_##name = { (queue_sz), sizeof(type), &os_mailQ_p_##name }
#endif

/**
 * @brief Access a Mail Queue Definition.
 * @param         name          name of the queue
 * @note CAN BE CHANGED: The parameter to \b osMailQ shall be consistent but the
                         macro body is implementation specific in every CMSIS-RTOS.
 */
#define osMailQ(name) &os_mailQ_##name

/**
 * @brief Create and Initialize a Mail Queue object.
 * @param[in]     queue_def     mail queue definition referenced with \ref osMailQ.
 * @param[in]     thread_id     thread ID (obtained by \ref osThreadCreate or \ref osThreadGetId) or NULL.
 * @return mail queue ID for reference by other functions or NULL in case of error.
 */
osMailQId osMailCreate (const osMailQDef_t *queue_def, osThreadId thread_id);

/**
 * @brief Allocate a memory block for mail from a mail memory pool.
 * @param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
 * @param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out
 * @return pointer to memory block that can be filled with mail or NULL in case of error.
 */
void *osMailAlloc (osMailQId queue_id, uint32_t millisec);

/**
 * @brief Allocate a memory block for mail from a mail memory pool and set memory block to zero.
 * @param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
 * @param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out
 * @return pointer to memory block that can be filled with mail or NULL in case of error.
 */
void *osMailCAlloc (osMailQId queue_id, uint32_t millisec);

/**
 * @brief Put a Mail into a Queue.
 * @param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
 * @param[in]     mail          pointer to memory with mail to put into a queue.
 * @return status code that indicates the execution status of the function.
 */
osStatus osMailPut (osMailQId queue_id, const void *mail);

/**
 * @brief Get a Mail from a Queue or timeout if Queue is empty.
 * @param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
 * @param[in]     millisec      \ref CMSIS_RTOS_TimeOutValue or 0 in case of no time-out.
 * @return event information that includes status code.
 */
osEvent osMailGet (osMailQId queue_id, uint32_t millisec);

/**
 * @brief Free a memory block by returning it to a mail memory pool.
 * @param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
 * @param[in]     mail          pointer to memory block that was obtained with \ref osMailGet.
 * @return status code that indicates the execution status of the function.
 */
osStatus osMailFree (osMailQId queue_id, void *mail);
/** @} cmsis_mailq */

/// Get the mail number in the mailbox
/// \param[in]     queue_id      mail queue ID obtained with \ref osMailCreate.
/// \return  the number in the mailbox
uint32_t osMailGetCount (osMailQId queue_id);

#ifdef __cplusplus
}
#endif

#endif
