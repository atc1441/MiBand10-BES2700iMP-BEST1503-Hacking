/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __APP_OVERLAY_H__
#define __APP_OVERLAY_H__

#include "plat_types.h"
#include "hal_overlay.h"
#include "hal_overlay_subsys.h"

#define app_overlay_load(id)                hal_overlay_load((enum HAL_OVERLAY_ID_T)id)
#define app_overlay_unload(id)              hal_overlay_unload((enum HAL_OVERLAY_ID_T)id)
#define app_overlay_get_text_size(id)       hal_overlay_get_text_size((enum HAL_OVERLAY_ID_T)id)
#define app_overlay_get_text_all_size       hal_overlay_get_text_all_size
#define app_overlay_get_text_address        hal_overlay_get_text_address
#define app_overlay_get_text_free_size(id)  hal_overlay_get_text_free_size((enum HAL_OVERLAY_ID_T)id)
#define app_overlay_get_text_free_addr(id)  hal_overlay_get_text_free_addr((enum HAL_OVERLAY_ID_T)id)

#define app_overlay_subsys_load(chip_id, id)         hal_overlay_subsys_load((enum OVERLAY_SUBSYS_T)chip_id, (enum HAL_OVERLAY_ID_T)id)
#define app_overlay_subsys_unload(chip_id, id)       hal_overlay_subsys_unload((enum OVERLAY_SUBSYS_T)chip_id, (enum HAL_OVERLAY_ID_T)id)

//#define app_overlay_is_used                 hal_overlay_is_used
enum APP_OVERLAY_SUBSYS_T {
    APP_OVERLAY_M55,
    APP_OVERLAY_HIFI4,

    APP_OVERLAY_SUBSYS_QTY,
};

enum APP_OVERLAY_ID_T {
    APP_OVERLAY_HFP = HAL_OVERLAY_ID_0,
    APP_OVERLAY_SPEECH_ALGO = HAL_OVERLAY_ID_0,
    APP_OVERLAY_A2DP = HAL_OVERLAY_ID_1,
    APP_OVERLAY_A2DP_LC3 = HAL_OVERLAY_ID_2,
    APP_OVERLAY_LC3 = HAL_OVERLAY_ID_2,
    APP_OVERLAY_LC3PLUS = HAL_OVERLAY_ID_3,
    APP_OVERLAY_AAC = HAL_OVERLAY_ID_4,
    APP_OVERLAY_A2DP_AAC = HAL_OVERLAY_ID_4,
    APP_OVERLAY_MPA = HAL_OVERLAY_ID_5,
#if	defined(A2DP_SCALABLE_ON)
    APP_OVERLAY_A2DP_SCALABLE= HAL_OVERLAY_ID_6,
#endif // A2DP_SCALABLE_ON
#if defined(A2DP_LHDC_ON)
    APP_OVERLAY_A2DP_LHDC = HAL_OVERLAY_ID_6,
#endif // A2DP_LHDC_ON
#if defined(A2DP_SOURCE_LHDC_ON)
    APP_OVERLAY_A2DP_LHDC_ENCODER = HAL_OVERLAY_ID_6,
#endif // A2DP_SOURCE_LHDC_ON
#if defined(A2DP_L2HC_ON)
	APP_OVERLAY_A2DP_L2HC = HAL_OVERLAY_ID_6,
#endif // A2DP_L2HC_ON
    APP_OVERLAY_WAV = HAL_OVERLAY_ID_6,
    APP_OVERLAY_MP3 = HAL_OVERLAY_ID_6,
#if defined(A2DP_LDAC_ON)
    APP_OVERLAY_A2DP_LDAC = HAL_OVERLAY_ID_7,
#endif
#if defined(A2DP_LHDCV5_ON)
    APP_OVERLAY_A2DP_LHDC_V5 = HAL_OVERLAY_ID_8,
#endif
#if defined(A2DP_SOURCE_LHDCV5_ON)
    APP_OVERLAY_A2DP_LHDC_V5_ENCODER = HAL_OVERLAY_ID_8,
#endif // A2DP_SOURCE_LHDCV5_ON
    APP_OVERLAY_OPUS = HAL_OVERLAY_ID_9,
    APP_OVERLAY_VORBIS = HAL_OVERLAY_ID_10,
    APP_OVERLAY_ALAC = HAL_OVERLAY_ID_11,
    APP_OVERLAY_FLAC = HAL_OVERLAY_ID_12,
    APP_OVERLAY_ID_QTY = HAL_OVERLAY_ID_QTY,
    APP_OVERLAY_ID_IN_CFG = HAL_OVERLAY_ID_IN_CFG,
};

enum APP_OVERLAY_SUBSYS_ID_T {
    APP_OVERLAY_SUBSYS_SBC = HAL_OVERLAY_ID_0,
#if defined(A2DP_AAC_ON)
    APP_OVERLAY_SUBSYS_AAC = HAL_OVERLAY_ID_1,
#endif
#if defined(A2DP_SCALABLE_ON)
    APP_OVERLAY_SUBSYS_SCALABLE= HAL_OVERLAY_ID_2,
#endif
#if defined(GAF_CODEC_CROSS_CORE)
    APP_OVERLAY_SUBSYS_AOB_LC3 = HAL_OVERLAY_ID_3,
#endif
#if defined(A2DP_LDAC_ON)
    APP_OVERLAY_SUBSYS_LDAC = HAL_OVERLAY_ID_3,
#endif
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
    APP_OVERLAY_SUBSYS_LHDC = HAL_OVERLAY_ID_3,
#endif
    APP_OVERLAY_SUBSYS_SCO      = HAL_OVERLAY_ID_4,
    APP_OVERLAY_SUBSYS_SPEECH_ALGO  = HAL_OVERLAY_ID_4,
    APP_OVERLAY_SUBSYS_SCO_CVSD = HAL_OVERLAY_ID_5,
    APP_OVERLAY_SUBSYS_SCO_MSBC = HAL_OVERLAY_ID_6,
    APP_OVERLAY_SUBSYS_SCO_XXXX = HAL_OVERLAY_ID_7,
    APP_OVERLAY_SUBSYS_ID_QTY = HAL_OVERLAY_ID_QTY,
    APP_OVERLAY_SUBSYS_ID_IN_CFG = HAL_OVERLAY_ID_IN_CFG,
};

#ifdef __cplusplus
extern "C" {
#endif

void app_overlay_select(enum APP_OVERLAY_ID_T id);

void app_overlay_unloadall(void);

void app_overlay_open(void);

void app_overlay_close(void);

enum APP_OVERLAY_ID_T app_get_current_overlay(void);

enum APP_OVERLAY_SUBSYS_ID_T app_get_current_overlay_subsys(enum APP_OVERLAY_SUBSYS_T subsys);

void app_overlay_subsys_select(enum APP_OVERLAY_SUBSYS_T subsys, enum APP_OVERLAY_SUBSYS_ID_T id);

void app_overlay_subsys_unloadall(enum APP_OVERLAY_SUBSYS_T subsys);

void app_overlay_subsys_open(void);

#ifdef __cplusplus
}
#endif

#endif
