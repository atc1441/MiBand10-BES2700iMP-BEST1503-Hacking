/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
#ifndef __GAF_DBG_INC__
#define __GAF_DBG_INC__

#include "bluetooth.h"

#include "hal_trace.h"

/*ENUM*/
#define GAF_LOG_LEVEL_CRITICAL  (0)
#define GAF_LOG_LEVEL_ERROR     (1)
#define GAF_LOG_LEVEL_WARN      (2)
#define GAF_LOG_LEVEL_INFO      (3)
#define GAF_LOG_LEVEL_DEBUG     (4)

#define GAF_LOG_LEVEL_MAX       (5)

/*DEFINE*/
#define GAF_LOG_LEVEL           GAF_LOG_LEVEL_DEBUG

#ifndef GAF_LOG_MODULE
#define GAF_LOG_MODULE          "GAF_LOG_UNDEF"
#endif /// GAF_LOG_MODULE

#if defined (BUILD_BTH_ROM)
#define GAF_PRINTF(lvl, str, ...)\
                                gaf_log_printf(lvl, str, ##__VA_ARGS__)
#else
#define GAF_PRINTF(lvl, str, ...)\
                                TRACE(TR_MOD(BLESTACK), str, ##__VA_ARGS__)
#endif

#if (GAF_LOG_LEVEL >= GAF_LOG_LEVEL_DEBUG)
#define GAF_LOG_D(str, ...)     GAF_PRINTF(GAF_LOG_LEVEL_DEBUG, "[D][%s][%d]"str, GAF_LOG_MODULE, __LINE__, ##__VA_ARGS__)
#else
#define GAF_LOG_D(str, ...)
#endif

#if (GAF_LOG_LEVEL >= GAF_LOG_LEVEL_INFO)
#define GAF_LOG_I(str, ...)     GAF_PRINTF(GAF_LOG_LEVEL_INFO, "[I][%s][%d]"str, GAF_LOG_MODULE, __LINE__, ##__VA_ARGS__)
#else
#define GAF_LOG_I(str, ...)
#endif

#if (GAF_LOG_LEVEL >= GAF_LOG_LEVEL_WARN)
#define GAF_LOG_W(str, ...)     GAF_PRINTF(GAF_LOG_LEVEL_WARN, "[W][%s][%d]"str, GAF_LOG_MODULE, __LINE__, ##__VA_ARGS__)
#else
#define GAF_LOG_W(str, ...)
#endif

#if (GAF_LOG_LEVEL >= GAF_LOG_LEVEL_ERROR)
#define GAF_LOG_E(str, ...)     GAF_PRINTF(GAF_LOG_LEVEL_ERROR, "[E][%s][%d]"str, GAF_LOG_MODULE, __LINE__, ##__VA_ARGS__)
#else
#define GAF_LOG_E(str, ...)
#endif

#if (GAF_LOG_LEVEL >= GAF_LOG_LEVEL_DEBUG)
#define GAF_DUMP8(str, ...)     GAF_LOG_D("[DUMP]"); gaf_log_dump8(str, ##__VA_ARGS__);
#define GAF_DUMP16(str, ...)    GAF_LOG_D("[DUMP]"); gaf_log_dump16(str, ##__VA_ARGS__);
#define GAF_DUMP32(str, ...)    GAF_LOG_D("[DUMP]"); gaf_log_dump32(str, ##__VA_ARGS__);
#else
#define GAF_DUMP8(str, ...)
#define GAF_DUMP16(str, ...)
#define GAF_DUMP32(str, ...)
#endif

int gaf_log_printf(uint8_t log_lvl, const char *format, ...);

int gaf_log_dump8(const char *format, const void *buffer, unsigned int count);

int gaf_log_dump16(const char *format, const void *buffer, unsigned int count);

int gaf_log_dump32(const char *format, const void *buffer, unsigned int count);

#endif /// __GAF_DBG_INC__
