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

#ifndef __APP_IBRT_DBG_H__
#define __APP_IBRT_DBG_H__
#include "hal_trace.h"

#ifdef __cplusplus
extern "C"{
#endif

#define APP_IBRT_LOG_LEVER   TR_LEVEL_INFO

#define LOG_V(str, ...)    if (APP_IBRT_LOG_LEVER >= TR_LEVEL_VERBOSE) TR_VERBOSE(TR_MOD(APP_IBRT), str, ##__VA_ARGS__)
#define LOG_D(str, ...)    if (APP_IBRT_LOG_LEVER >= TR_LEVEL_DEBUG) TR_DEBUG(TR_MOD(APP_IBRT), str, ##__VA_ARGS__)
#define LOG_I(str, ...)    if (APP_IBRT_LOG_LEVER >= TR_LEVEL_INFO) TR_INFO(TR_MOD(APP_IBRT), str, ##__VA_ARGS__)
#define LOG_W(str, ...)    if (APP_IBRT_LOG_LEVER >= TR_LEVEL_WARN) TR_WARN(TR_MOD(APP_IBRT), str, ##__VA_ARGS__)
#define LOG_E(str, ...)    if (APP_IBRT_LOG_LEVER >= TR_LEVEL_ERROR) TR_ERROR(TR_MOD(APP_IBRT), str, ##__VA_ARGS__)
#define LOG_IMM(str, ...)  if (APP_IBRT_LOG_LEVER >= TR_LEVEL_INFO) TR_INFO(TR_ATTR_IMM, str, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __APP_UI_DBG_H__ */

