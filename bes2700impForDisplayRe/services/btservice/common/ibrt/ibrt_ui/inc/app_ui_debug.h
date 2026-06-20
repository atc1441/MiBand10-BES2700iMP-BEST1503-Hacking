/***************************************************************************
 *
 * Copyright 2015-2020 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means), or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright), trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#ifndef __APP_UI_DBG_H__
#define __APP_UI_DBG_H__

#include "hal_trace.h"

#define UX_SVC_TRACE_ENABLE

#define MODULE_TRACE_LEVEL  TR_LEVEL_INFO

#ifdef __cplusplus
extern "C"{
#endif

#ifdef UX_SVC_TRACE_ENABLE
#define LOG_V(str, ...)     if (MODULE_TRACE_LEVEL >= TR_LEVEL_VERBOSE)     TR_VERBOSE(TR_MOD(APP_UX), str, ##__VA_ARGS__)
#define LOG_D(str, ...)     if (MODULE_TRACE_LEVEL >= TR_LEVEL_DEBUG)       TR_DEBUG(TR_MOD(APP_UX), str, ##__VA_ARGS__)
#define LOG_I(str, ...)     if (MODULE_TRACE_LEVEL >= TR_LEVEL_INFO)        TR_INFO(TR_MOD(APP_UX), str, ##__VA_ARGS__)
#define LOG_W(str, ...)     if (MODULE_TRACE_LEVEL >= TR_LEVEL_WARN)        TR_WARN(TR_MOD(APP_UX), str, ##__VA_ARGS__)
#define LOG_E(str, ...)     if (MODULE_TRACE_LEVEL >= TR_LEVEL_ERROR)       TR_ERROR(TR_MOD(APP_UX), str, ##__VA_ARGS__)
#define LOG_IMM(str, ...)   TR_INFO(TR_ATTR_IMM, str, ##__VA_ARGS__)
#else
#define LOG_V(str, ...)
#define LOG_D(str, ...)
#define LOG_I(str, ...)
#define LOG_W(str, ...)
#define LOG_E(str, ...)
#define LOG_IMM(str, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __APP_UI_DBG_H__ */

