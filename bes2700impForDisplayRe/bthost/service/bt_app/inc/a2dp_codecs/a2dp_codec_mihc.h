/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#ifndef __A2DP_CODEC_MIHC_H__
#define __A2DP_CODEC_MIHC_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "a2dp_api.h"

#define A2DP_MIHC_VENDOR_ID     (0x0000038F)
#define A2DP_MIHC_CODEC_ID      (0x4D31)

// Version (Determines the min-bitrate)
#define A2DP_MIHC_VERSION_00    (0x00)
#define A2DP_MIHC_VERSION_01    (0x01)
#define A2DP_MIHC_VERSION_02    (0x02)
#define A2DP_MIHC_VERSION_04    (0x04)
#define A2DP_MIHC_VERSION_08    (0x08)
#define A2DP_MIHC_VERSION_10    (0x10)
#define A2DP_MIHC_VERSION_20    (0x20)
#define A2DP_MIHC_VERSION_40    (0x40)
#define A2DP_MIHC_VERSION_80    (0x80)

// Sample rate (Hz)
#define A2DP_MIHC_SR_48000      (0x04)
#define A2DP_MIHC_SR_44100      (0x08)
#define A2DP_MIHC_SR_32000      (0x10)
#define A2DP_MIHC_SR_24000      (0x20)
#define A2DP_MIHC_SR_16000      (0x40)
#define A2DP_MIHC_SR_DATA(X)    ((X) & (A2DP_MIHC_SR_48000|A2DP_MIHC_SR_44100|A2DP_MIHC_SR_32000|A2DP_MIHC_SR_24000|A2DP_MIHC_SR_16000))

// Constant bit rate
#define A2DP_MIHC_CBR           (0x01)

// Frame Duration
#define A2DP_MIHC_FD_10MS       (0x04)
#define A2DP_MIHC_FD_5MS        (0x08)
#define A2DP_MIHC_FD_DATA(X)    ((X) & (A2DP_MIHC_FD_10MS|A2DP_MIHC_FD_5MS))

// Bit deepth (bit)
#define A2DP_MIHC_BD_32         (0x20)
#define A2DP_MIHC_BD_24         (0x40)
#define A2DP_MIHC_BD_16         (0x80)
#define A2DP_MIHC_BD_DATA(X)    ((X) & (A2DP_MIHC_BD_32|A2DP_MIHC_BD_24|A2DP_MIHC_BD_16))

// Channel mode
#define A2DP_MIHC_CM_STEREO     (0X01)
#define A2DP_MIHC_CM_MONO       (0x02)
#define A2DP_MIHC_CM_DATA(X)    ((X) & (A2DP_MIHC_CM_STEREO|A2DP_MIHC_CM_MONO))

// MIHC mode
#define A2DP_MIHC_MODE_ADATPIVE     (0x10)
#define A2DP_MIHC_MODE_LOW_LATENCY  (0x20)
#define A2DP_MIHC_MODE_LOSSLESS     (0x40)
#define A2DP_MIHC_MODE_LOSSY        (0x80)
#define A2DP_MIHC_MODE_DATA(X)      ((X) & (A2DP_MIHC_MODE_ADATPIVE|A2DP_MIHC_MODE_LOW_LATENCY|A2DP_MIHC_MODE_LOSSLESS|A2DP_MIHC_MODE_LOSSY))

// Min-bitrate (version-dependent)
#define A2DP_MIHC_MIN_BR_1280       (0x80)
#define A2DP_MIHC_MIN_BR_960        (0x40)
#define A2DP_MIHC_MIN_BR_640        (0x20)
#define A2DP_MIHC_MIN_BR_320        (0x10)
#define A2DP_MIHC_MIN_BR_DEFAULT    (0x10) // version_00 default use minimum min-birate

#define A2DP_MIHC_MD                (0x80)

#define A2DP_MIHC_ALL_BITS_ZERO     (0x00)

#if defined(A2DP_MIHC_ON)
void a2dp_codec_mihc_common_init(void);
bt_status_t a2dp_codec_mihc_init(int index);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* __A2DP_CODEC_MIHC_H__ */