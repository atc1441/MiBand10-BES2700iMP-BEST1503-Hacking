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
#ifndef __HAL_TRACE_MOD_H__
#define __HAL_TRACE_MOD_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TR_MOD(m)                           TR_ATTR_MOD(TR_MODULE_ ## m)

#define _TR_MODULE_DEF_A(p, m)              p ## m
#define _TR_MODULE_DEF(m)                   _TR_MODULE_DEF_A(TR_MODULE_, m)

#define _TR_MODULE_LIST                     \
    _TR_MODULE_DEF(NONE),                   \
    _TR_MODULE_DEF(HAL),                    \
    _TR_MODULE_DEF(DRVANA),                 \
    _TR_MODULE_DEF(DRVCODEC),               \
    _TR_MODULE_DEF(DRVBT),                  \
    _TR_MODULE_DEF(DRVFLS),                 \
    _TR_MODULE_DEF(DRVSEC),                 \
    _TR_MODULE_DEF(DRVUSB),                 \
    _TR_MODULE_DEF(AUDFLG),                 \
    _TR_MODULE_DEF(MAIN),                   \
    _TR_MODULE_DEF(RT_OS),                  \
    _TR_MODULE_DEF(BTPRF),                  \
    _TR_MODULE_DEF(BLEPRF),                 \
    _TR_MODULE_DEF(BTAPP),                  \
    _TR_MODULE_DEF(BLEAPP),                 \
    _TR_MODULE_DEF(BLESTACK),               \
    _TR_MODULE_DEF(TWSAPP),                 \
    _TR_MODULE_DEF(IBRTAPP),                \
    _TR_MODULE_DEF(APPMAIN),                \
    _TR_MODULE_DEF(APPTHREAD),              \
    _TR_MODULE_DEF(PLAYER),                 \
    _TR_MODULE_DEF(TEST),                   \
    _TR_MODULE_DEF(AUD),                    \
    _TR_MODULE_DEF(OTA),                    \
    _TR_MODULE_DEF(NV_SEC),                 \
    _TR_MODULE_DEF(AI_GVA),                 \
    _TR_MODULE_DEF(AI_VOC),                 \
    _TR_MODULE_DEF(AI_AMA),                 \
    _TR_MODULE_DEF(AI_GMA),                 \
    _TR_MODULE_DEF(AI_BIXBY),               \
    _TR_MODULE_DEF(AOB_SM),                 \
    _TR_MODULE_DEF(SPEECH),                 \
    _TR_MODULE_DEF(WALKIE_TALKIE),          \
    _TR_MODULE_DEF(VOICE_COMPRESS),         \
    _TR_MODULE_DEF(BT_GATT),                \
    _TR_MODULE_DEF(APP_UX),                 \
    _TR_MODULE_DEF(IBRT_CORE),              \
    _TR_MODULE_DEF(APP_IBRT),               \
    _TR_MODULE_DEF(CC_DECODE),              \
    _TR_MODULE_DEF(ESHELL),                 \
    _TR_MODULE_DEF(SPA),                    \
    _TR_MODULE_DEF(BTSPK),                  \
    _TR_MODULE_DEF(BIS_T),                  \
    _TR_MODULE_DEF(BT_SVC),                 \
    _TR_MODULE_DEF(INFO),              		\
    _TR_MODULE_DEF(LCB),
enum TR_MODULE_T {
    _TR_MODULE_LIST

    // Should <= 128
    TR_MODULE_QTY,
};

const char *hal_trace_get_log_module_desc(enum TR_MODULE_T module);

#ifdef __cplusplus
}
#endif

#endif
