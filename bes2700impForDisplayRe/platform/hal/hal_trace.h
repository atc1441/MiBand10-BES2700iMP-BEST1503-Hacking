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
#ifndef __HAL_TRACE_H__
#define __HAL_TRACE_H__
#define __INCLUDE_ASSERT_H
#ifndef __ASSEMBLY__
#ifdef __cplusplus
extern "C" {
#endif

#include "plat_addr_map.h"
#include "plat_types.h"
#include "hal_trace_mod.h"


#define printl(...) 	            TRACE(1, ##__VA_ARGS__)
#define print_r(a,b)                DUMP8("0x%02x ",a,b)
#define print_lcb(str,...)          TRACE(TR_MOD(LCB),"[LCB]------->>>""" str, ##__VA_ARGS__)
#define print_info(str,...)         TRACE(TR_MOD(INFO),"%s(%d)-[%s] """ str,__FILE__,__LINE__,__func__, ##__VA_ARGS__)


// #define AUDIO_DEBUG
// #define INTERSYS_RAW_DATA_ONLY

#if defined(INTERSYS_RAW_DATA_ONLY)
#define NO_REL_TRACE
#endif

/*
 * Total number of core registers stored
 */
#define CRASH_DUMP_REGISTERS_NUM            17
#define CRASH_DUMP_REGISTERS_NUM_BYTES      (CRASH_DUMP_REGISTERS_NUM * 4)

/*
 * Number bytes to store from stack
 *   - this is total, not per PSP/MSP
 */
#define CRASH_DUMP_STACK_NUM_BYTES          384

// Log Attributes
#define TR_ATTR_ARG_NUM_SHIFT               0
#define TR_ATTR_ARG_NUM_MASK                (0xF << TR_ATTR_ARG_NUM_SHIFT)
#define TR_ATTR_ARG_NUM(n)                  BITFIELD_VAL(TR_ATTR_ARG_NUM, n)
#define TR_ATTR_LEVEL_SHIFT                 4
#define TR_ATTR_LEVEL_MASK                  (0x7 << TR_ATTR_LEVEL_SHIFT)
#define TR_ATTR_LEVEL(n)                    BITFIELD_VAL(TR_ATTR_LEVEL, n)
#define TR_ATTR_MOD_SHIFT                   7
#define TR_ATTR_MOD_MASK                    (0x7F << TR_ATTR_MOD_SHIFT)
#define TR_ATTR_MOD(n)                      BITFIELD_VAL(TR_ATTR_MOD, n)
#define TR_ATTR_IMM                         (1 << 14)
#define TR_ATTR_NO_LF                       (1 << 15)
#define TR_ATTR_NO_TS                       (1 << 16)
#define TR_ATTR_NO_ID                       (1 << 17)

// Count variadic argument number
#define _VAR_ARG_17(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, ...) a17
#define COUNT_ARG_NUM(...)                  _VAR_ARG_17(unused, ##__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#if defined(TRACE_STR_SECTION) && !(defined(ROM_BUILD) || defined(PROGRAMMER) || defined(FPGA))
#define CONCAT_(x,y)                        x##y
#define CONCATS(x,y)                        CONCAT_(x,y)
#define __trcname                           CONCATS(__trc, __LINE__)
#define TRC_STR_LOC                         __attribute__((section(TO_STRING(CONCATS(.trc_str,__LINE__)))))
#define TRC_STR(s)                          (({ static const char TRC_STR_LOC __trcname[] = (s); __trcname; }))
#else
#define TRC_STR_LOC
#define TRC_STR(s)                          (s)
#endif

#define TR_DUMMY(attr, str, ...)            hal_trace_dummy(str, ##__VA_ARGS__)

#if (defined(DEBUG) || defined(REL_TRACE_ENABLE)) && !defined(NO_REL_TRACE)
#define REL_LOG(attr, str, ...)             hal_trace_printf(((attr) & ~TR_ATTR_ARG_NUM_MASK) | \
                                                            TR_ATTR_ARG_NUM(COUNT_ARG_NUM(unused, ##__VA_ARGS__)), \
                                                            TRC_STR(str), ##__VA_ARGS__)
#define REL_LOG_RAW_OUTPUT(str, len)        hal_trace_output(str, len)
#define REL_LOG_OUTPUT_LINEFEED()           hal_trace_output_linefeed()
#define REL_LOG_FLUSH()                     hal_trace_flush_buffer()
#define REL_LOG_SEND()                      hal_trace_send()
#define REL_DUMP8(str, buf, cnt)            hal_trace_dump(str, sizeof(uint8_t), cnt, buf)
#define REL_DUMP16(str, buf, cnt)           hal_trace_dump(str, sizeof(uint16_t), cnt, buf)
#define REL_DUMP32(str, buf, cnt)           hal_trace_dump(str, sizeof(uint32_t), cnt, buf)
#else
#define REL_LOG(attr, str, ...)             hal_trace_dummy(str, ##__VA_ARGS__)
#define REL_LOG_RAW_OUTPUT(str, len)        hal_trace_dummy((const char *)str, len)
#define REL_LOG_OUTPUT_LINEFEED()           hal_trace_dummy(NULL)
#define REL_LOG_FLUSH()                     hal_trace_dummy(NULL)
#define REL_LOG_SEND()                      hal_trace_dummy(NULL)
#define REL_DUMP8(str, buf, cnt)            hal_dump_dummy(str, buf, cnt)
#define REL_DUMP16(str, buf, cnt)           hal_dump_dummy(str, buf, cnt)
#define REL_DUMP32(str, buf, cnt)           hal_dump_dummy(str, buf, cnt)
#endif

#if (!defined(DEBUG) && defined(REL_TRACE_ENABLE)) && !defined(NO_TRACE)
// To avoid warnings on unused variables
#define NORM_LOG(num,str, ...)              hal_trace_dummy(str, ##__VA_ARGS__)
#define NORM_LOG_RAW_OUTPUT(str, len)       hal_trace_dummy((const char *)str, len)
#define NORM_LOG_OUTPUT_LINEFEED()          hal_trace_dummy(NULL)
#define NORM_LOG_FLUSH()                    hal_trace_dummy(NULL)
#define NORM_LOG_SEND()                     hal_trace_dummy(NULL)
#define DUMP8(str, buf, cnt)                hal_dump_dummy(str, buf, cnt)
#define DUMP16(str, buf, cnt)               hal_dump_dummy(str, buf, cnt)
#define DUMP32(str, buf, cnt)               hal_dump_dummy(str, buf, cnt)
#else
#define NORM_LOG                            REL_LOG
#define NORM_LOG_RAW_OUTPUT                 REL_LOG_RAW_OUTPUT
#define NORM_LOG_OUTPUT_LINEFEED            REL_LOG_OUTPUT_LINEFEED
#define NORM_LOG_FLUSH                      REL_TRACE_FLUSH
#define NORM_LOG_SEND                       REL_LOG_SEND
#define DUMP8                               REL_DUMP8
#define DUMP16                              REL_DUMP16
#define DUMP32                              REL_DUMP32
#endif

#define R_TR_CRITICAL(attr, str, ...)       REL_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_CRITICAL), \
                                                    str, ##__VA_ARGS__)
#define R_TR_ERROR(attr, str, ...)          REL_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_ERROR), \
                                                    str, ##__VA_ARGS__)
#define R_TR_WARN(attr, str, ...)           REL_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_WARN), \
                                                    str, ##__VA_ARGS__)
#define R_TR_NOTIF(attr, str, ...)          REL_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_NOTIF), \
                                                    str, ##__VA_ARGS__)
#define R_TR_INFO(attr, str, ...)           REL_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_INFO), \
                                                    str, ##__VA_ARGS__)
#define R_TR_DEBUG(attr, str, ...)          REL_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_DEBUG), \
                                                    str, ##__VA_ARGS__)
#define R_TR_VERBOSE(attr, str, ...)        REL_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_VERBOSE), \
                                                    str, ##__VA_ARGS__)

#define TR_CRITICAL(attr, str, ...)         NORM_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_CRITICAL), \
                                                    str, ##__VA_ARGS__)
#define TR_ERROR(attr, str, ...)            NORM_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_ERROR), \
                                                    str, ##__VA_ARGS__)
#define TR_WARN(attr, str, ...)             NORM_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_WARN), \
                                                    str, ##__VA_ARGS__)
#define TR_NOTIF(attr, str, ...)            NORM_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_NOTIF), \
                                                    str, ##__VA_ARGS__)
#define TR_INFO(attr, str, ...)             NORM_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_INFO), \
                                                    str, ##__VA_ARGS__)
#define TR_DEBUG(attr, str, ...)            NORM_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_DEBUG), \
                                                    str, ##__VA_ARGS__)
#define TR_VERBOSE(attr, str, ...)          NORM_LOG(((attr) & ~TR_ATTR_LEVEL_MASK) | TR_ATTR_LEVEL(TR_LEVEL_VERBOSE), \
                                                    str, ##__VA_ARGS__)

#define REL_TRACE(attr, str, ...)           R_TR_NOTIF(attr, str, ##__VA_ARGS__)
#define REL_TRACE_IMM(attr, str, ...)       R_TR_NOTIF((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define REL_TRACE_NOCRLF(attr, str, ...)    R_TR_NOTIF((attr) | TR_ATTR_NO_LF, str, ##__VA_ARGS__)
#define REL_TRACE_NOTS(attr, str, ...)      R_TR_NOTIF((attr) | TR_ATTR_NO_TS, str, ##__VA_ARGS__)
#define REL_TRACE_IMM_NOTS(attr, str, ...)  R_TR_NOTIF((attr) | TR_ATTR_IMM | TR_ATTR_NO_TS, str, ##__VA_ARGS__)
#define REL_TRACE_NOCRLF_NOTS(attr, str, ...) R_TR_NOTIF((attr) | TR_ATTR_NO_LF | TR_ATTR_NO_TS, str, ##__VA_ARGS__)
#define REL_FUNC_ENTRY_TRACE()              R_TR_NOTIF(1, "%s", __FUNCTION__)
#define REL_TRACE_OUTPUT(str, len)          REL_LOG_RAW_OUTPUT(str, len)
#define REL_TRACE_OUTPUT_LINEFEED()         REL_LOG_OUTPUT_LINEFEED()
#define REL_TRACE_FLUSH()                   REL_LOG_FLUSH()

#define TRACE(attr, str, ...)               TR_INFO(attr, str, ##__VA_ARGS__)
#define TRACE_IMM(attr, str, ...)           TR_INFO((attr) | TR_ATTR_IMM, str, ##__VA_ARGS__)
#define TRACE_NOCRLF(attr, str, ...)        TR_INFO((attr) | TR_ATTR_NO_LF, str, ##__VA_ARGS__)
#define TRACE_NOTS(attr, str, ...)          TR_INFO((attr) | TR_ATTR_NO_TS, str, ##__VA_ARGS__)
#define TRACE_NOCRLF_NOTS(attr, str, ...)   TR_INFO((attr) | TR_ATTR_NO_LF | TR_ATTR_NO_TS, str, ##__VA_ARGS__)
#define FUNC_ENTRY_TRACE()                  TR_INFO(1, "%s", __FUNCTION__)
#define TRACE_OUTPUT(str, len)              NORM_LOG_RAW_OUTPUT(str, len)
#define TRACE_OUTPUT_LINEFEED()             NORM_LOG_OUTPUT_LINEFEED()
#define TRACE_FLUSH()                       NORM_LOG_FLUSH()
#define TRACE_SEND()                        NORM_LOG_SEND()
#ifdef BES_AUTOMATE_TEST
#define AUTO_TEST_TRACE(attr, str, ...)     TR_INFO(attr, "_AT_" str, ##__VA_ARGS__)
#else
#define AUTO_TEST_TRACE(attr, str, ...)     TR_INFO(attr, str, ##__VA_ARGS__)
#endif

#define TRACE_DUMMY(attr, str, ...)         TR_DUMMY(attr, str, ##__VA_ARGS__)

#if defined(__NuttX__)
#include <syslog.h>
void __assert(FAR const char *filename, int linenum, FAR const char *msg);
#undef ASSERT
#define ASSERT(cond, str, ...)      { if (!(cond)) { R_TR_ERROR(0,str,##__VA_ARGS__); __assert(__FUNCTION__,__LINE__, #cond); } }
#define ASSERT_DUMP_ARGS            const char *fmt, ...
#define ASSERT_FMT_ARG_IDX          1
#else

#if (defined(DEBUG) || defined(REL_TRACE_ENABLE)) && defined(ASSERT_SHOW_FILE_FUNC)
#define ASSERT(cond, str, ...)      { if (!(cond)) { hal_trace_assert_dump(__FILE__, __FUNCTION__, __LINE__, str, ##__VA_ARGS__); } }
#define ASSERT_DUMP_ARGS            const char *file, const char *func, unsigned int line, const char *fmt, ...
#define ASSERT_FMT_ARG_IDX          4
#elif (defined(DEBUG) || defined(REL_TRACE_ENABLE)) && defined(ASSERT_SHOW_FILE)
#define ASSERT(cond, str, ...)      { if (!(cond)) { hal_trace_assert_dump(__FILE__, __LINE__, str, ##__VA_ARGS__); } }
#define ASSERT_DUMP_ARGS            const char *file, const char *func, unsigned int line, const char *fmt, ...
#define ASSERT_FMT_ARG_IDX          4
#elif (defined(DEBUG) || defined(REL_TRACE_ENABLE)) && defined(ASSERT_SHOW_FUNC)
#define ASSERT(cond, str, ...)      { if (!(cond)) { hal_trace_assert_dump(__FUNCTION__, __LINE__, str, ##__VA_ARGS__); } }
#define ASSERT_DUMP_ARGS            const char *scope, unsigned int line, const char *fmt, ...
#define ASSERT_FMT_ARG_IDX          3
#elif (defined(DEBUG) || defined(REL_TRACE_ENABLE))
#define ASSERT(cond, str, ...)      { if (!(cond)) { hal_trace_assert_dump(str, ##__VA_ARGS__); } }
#define ASSERT_DUMP_ARGS            const char *fmt, ...
#define ASSERT_FMT_ARG_IDX          1
#else
#define ASSERT(cond, str, ...)      { if (!(cond)) { hal_trace_dummy(str, ##__VA_ARGS__); hal_trace_assert_dump(NULL); } }
#define ASSERT_DUMP_ARGS            const char *fmt
#define ASSERT_FMT_ARG_IDX          0
#endif

#endif

#if (defined(DEBUG) || defined(REL_TRACE_ENABLE))
#define TRACE_FUNC_DECLARE(d, r)    d
#else
#ifndef TRACE_FUNC_SPEC
#define TRACE_FUNC_SPEC             static inline
#endif
#define TRACE_FUNC_DECLARE(d, r)    TRACE_FUNC_SPEC d { r; }
#endif

#if defined(__GNUC__) && !defined(NO_CHK_TRC_FMT)
#define TRC_FMT_CHK(sidx, vaidx)    __attribute__((format(printf, (sidx), (vaidx))))
#else
#define TRC_FMT_CHK(sidx, vaidx)
#endif

#define ASSERT_NODUMP(cond)         { if (!(cond)) { SAFE_PROGRAM_STOP(); } }

#ifdef CHIP_BEST1000
// Avoid CPU instruction fetch blocking the system bus on BEST1000
#define SAFE_PROGRAM_STOP()         do { asm volatile("nop; nop; nop; nop"); } while (1)
#else
#define SAFE_PROGRAM_STOP()         do { } while (1)
#endif

/// definitions for easier debug
#define _ENUM_DEF(prefix, detail)                     prefix ## detail

enum HAL_TRACE_TRANSPORT_T {
    HAL_TRACE_TRANSPORT_NULL,
#ifdef CHIP_HAS_USB
    HAL_TRACE_TRANSPORT_USB,
#endif
    HAL_TRACE_TRANSPORT_UART0,
#ifdef UART1_BASE
    HAL_TRACE_TRANSPORT_UART1,
#endif
#ifdef UART2_BASE
    HAL_TRACE_TRANSPORT_UART2,
#endif

    HAL_TRACE_TRANSPORT_QTY
};

enum HAL_TRACE_STATE_T {
    HAL_TRACE_STATE_CRASH_ASSERT_START,
    HAL_TRACE_STATE_CRASH_FAULT_START,
    HAL_TRACE_STATE_CRASH_END,

    HAL_TRACE_STATE_CRASH_UNINITIALIZED = 0xFF,
};

enum HAL_TRACE_BUF_STATE_T {
    HAL_TRACE_BUF_STATE_FLUSH,
    HAL_TRACE_BUF_STATE_NEAR_FULL,
    HAL_TRACE_BUF_STATE_FULL,
};

enum HAL_TRACE_CRASH_DUMP_MODULE_T {
    HAL_TRACE_CRASH_DUMP_MODULE_SYS = 0,
    HAL_TRACE_CRASH_DUMP_MODULE_RMT_TRC,
    HAL_TRACE_CRASH_DUMP_MODULE_BTH,
    HAL_TRACE_CRASH_DUMP_MODULE_BT,

    HAL_TRACE_CRASH_DUMP_MODULE_END,
};

enum HAL_TRACE_APP_REG_ID_T {
    HAL_TRACE_APP_REG_ID_0,
    HAL_TRACE_APP_REG_ID_1,
    HAL_TRACE_APP_REG_ID_2,

    HAL_TRACE_APP_REG_ID_QTY,
};

enum TR_LEVEL_T {
    TR_LEVEL_CRITICAL   = 0,
    TR_LEVEL_ERROR      = 1,
    TR_LEVEL_WARN       = 2,
    TR_LEVEL_NOTIF      = 3,
    TR_LEVEL_INFO       = 4,
    TR_LEVEL_DEBUG      = 5,
    TR_LEVEL_VERBOSE    = 6,

    TR_LEVEL_QTY,
};

typedef void (*HAL_TRACE_CRASH_DUMP_CB_T)(void);

typedef void (*HAL_TRACE_APP_NOTIFY_T)(enum HAL_TRACE_STATE_T state);

typedef void (*HAL_TRACE_APP_OUTPUT_T)(const unsigned char *buf, unsigned int buf_len);

typedef void (*HAL_TRACE_BUF_CTRL_T)(enum HAL_TRACE_BUF_STATE_T buf_ctrl);

typedef int (*HAL_TRACE_GLOBAL_TAG_CB_T)(char *buf, unsigned int buf_len);

typedef void (*HAL_TRACE_SAVE_PLAYBACK_DEVICE_T)(void);

int hal_trace_open(enum HAL_TRACE_TRANSPORT_T transport);

TRACE_FUNC_DECLARE(int hal_trace_switch(enum HAL_TRACE_TRANSPORT_T transport), return 0);

TRACE_FUNC_DECLARE(int hal_trace_close(void), return 0);

TRACE_FUNC_DECLARE(enum HAL_TRACE_TRANSPORT_T hal_trace_get_transport(void), return HAL_TRACE_TRANSPORT_QTY);

TRACE_FUNC_DECLARE(void hal_trace_output_enable(bool en), return);

TRACE_FUNC_DECLARE(int hal_trace_enable_log_module(enum TR_MODULE_T module), return 0);

TRACE_FUNC_DECLARE(int hal_trace_disable_log_module(enum TR_MODULE_T module), return 0);

TRACE_FUNC_DECLARE(int hal_trace_set_log_module(const uint32_t *map, uint32_t word_cnt), return 0);

TRACE_FUNC_DECLARE(int hal_trace_set_log_level(enum TR_LEVEL_T level), return 0);

TRACE_FUNC_DECLARE(void hal_trace_get_history_buffer(const uint8_t **buf1, uint32_t *len1, const uint8_t **buf2, uint32_t *len2), \
    { if (buf1) { *buf1 = NULL; } if (len1) { *len1 = 0; } if (buf2) { *buf2 = NULL; } if (len2) { *len2 = 0; } });

TRACE_FUNC_DECLARE(int hal_trace_output(const unsigned char *buf, unsigned int buf_len), return 0);

TRACE_FUNC_DECLARE(int hal_trace_output_linefeed(void), return 0);

TRC_FMT_CHK(2, 3)
TRACE_FUNC_DECLARE(int hal_trace_printf(uint32_t attr, const char *fmt, ...), return 0);

TRACE_FUNC_DECLARE(int hal_trace_dump(const char *fmt, unsigned int size,  unsigned int count, const void *buffer), return 0);

TRACE_FUNC_DECLARE(int hal_trace_busy(void), return 0);

TRACE_FUNC_DECLARE(int hal_trace_pause(void), return 0);

TRACE_FUNC_DECLARE(int hal_trace_continue(void), return 0);

TRACE_FUNC_DECLARE(int hal_trace_flush_buffer(void), return 0);

TRACE_FUNC_DECLARE(int hal_trace_flush_output(void), return 0);

TRACE_FUNC_DECLARE(void hal_trace_send(void), return);

TRACE_FUNC_DECLARE(int hal_trace_crash_dump_register(enum HAL_TRACE_CRASH_DUMP_MODULE_T module, HAL_TRACE_CRASH_DUMP_CB_T cb), return 0);

TRACE_FUNC_DECLARE(void hal_trace_crash_dump_callback(void), return);

TRACE_FUNC_DECLARE(int hal_trace_app_register(enum HAL_TRACE_APP_REG_ID_T id, HAL_TRACE_APP_NOTIFY_T notify_cb, HAL_TRACE_APP_OUTPUT_T output_cb), return 0);

TRACE_FUNC_DECLARE(void hal_trace_app_notify_callback(enum HAL_TRACE_STATE_T state), return);

TRACE_FUNC_DECLARE(void hal_trace_app_custom_register(enum HAL_TRACE_APP_REG_ID_T id, HAL_TRACE_APP_NOTIFY_T notify_cb, HAL_TRACE_APP_OUTPUT_T output_cb, HAL_TRACE_APP_OUTPUT_T crash_custom_cb), return);

TRACE_FUNC_DECLARE(void hal_trace_global_tag_register(HAL_TRACE_GLOBAL_TAG_CB_T tag_cb), return);

TRACE_FUNC_DECLARE(int hal_trace_open_cp(HAL_TRACE_BUF_CTRL_T buf_cb, HAL_TRACE_APP_NOTIFY_T notify_cb), return 0);

TRACE_FUNC_DECLARE(int hal_trace_close_cp(void), return 0);

TRACE_FUNC_DECLARE(void hal_trace_print_backtrace(uint32_t addr, uint32_t search_cnt, uint32_t print_cnt), return);

TRACE_FUNC_DECLARE(bool hal_trace_in_crash_dump(void), return false);

TRACE_FUNC_DECLARE (uint32_t hal_trace_get_uart_id(void), return 0);

TRACE_FUNC_DECLARE(uint32_t hal_trace_get_uart_baudrate(void), return 0);

TRC_FMT_CHK(1, 2)
inline int hal_trace_dummy(const char *fmt, ...) { return 0; }

inline int hal_dump_dummy(const char *fmt, ...) { return 0; }

#if (ASSERT_FMT_ARG_IDX > 0)
TRC_FMT_CHK(ASSERT_FMT_ARG_IDX, ASSERT_FMT_ARG_IDX + 1)
#endif
void NORETURN hal_trace_assert_dump(ASSERT_DUMP_ARGS);

int hal_trace_address_writable(uint32_t addr);

int hal_trace_address_executable(uint32_t addr);

int hal_trace_address_readable(uint32_t addr);

int format_string(char *buf, size_t size, const char *fmt, ...);

//==============================================================================
// AUDIO_DEBUG
//==============================================================================

#ifdef AUDIO_DEBUG
#define AUDIO_DEBUG_TRACE(str, ...)         hal_trace_printf(str, ##__VA_ARGS__)
#define AUDIO_DEBUG_DUMP(buf, cnt)          hal_trace_output(buf, cnt)
#endif


//==============================================================================
// INTERSYS_RAW_DATA_ONLY
//==============================================================================

#ifdef INTERSYS_RAW_DATA_ONLY
#define TRACE_RAW(str, ...)                 hal_trace_printf(str, ##__VA_ARGS__)
#define DUMP8_RAW(str, buf, cnt)            hal_trace_dump(str, sizeof(uint8_t), cnt, buf)
#endif

//==============================================================================
// TRACE RX
//==============================================================================

typedef unsigned int (*HAL_TRACE_RX_CALLBACK_T)(unsigned char *buf, unsigned int len);

TRACE_FUNC_DECLARE(int hal_trace_rx_open(unsigned char *buf, unsigned int len, HAL_TRACE_RX_CALLBACK_T rx_callback), return 0);

TRACE_FUNC_DECLARE(int hal_trace_rx_close(void), return 0);

TRACE_FUNC_DECLARE(int hal_trace_rx_sleep(void), return 0);

TRACE_FUNC_DECLARE(int hal_trace_rx_wakeup(void), return 0);

#ifdef assert
#undef assert
#endif
#ifdef NDEBUG
#  define assert(f)
#else
#  define assert(f) ASSERT(f,"%s:%d",__func__,__LINE__)
#endif
#ifdef __cplusplus
}
#endif
#endif
#endif
