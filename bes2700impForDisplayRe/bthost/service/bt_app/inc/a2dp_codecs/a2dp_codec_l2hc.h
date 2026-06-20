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
#ifndef __A2DP_CODEC_L2HC_H__
#define __A2DP_CODEC_L2HC_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "a2dp_api.h"

#define A2DP_L2HC_VENDOR_ID                   (0x00000CCF)
#define A2DP_L2HC_CODEC_ID                    (0xCA01)

// Version Capability: Octet6; b4~b7
#define A2DP_L2HC_VERS_CAP          (0x00)

// Resolution(bits)
#define A2DP_L2HC_RES_32            (0x04)
#define A2DP_L2HC_RES_24            (0x02)
#define A2DP_L2HC_RES_16            (0x01)
#define A2DP_L2HC_RES_DATA(X)        (X & (A2DP_L2HC_RES_32|A2DP_L2HC_RES_24|A2DP_L2HC_RES_16))
// Sample rate(Hz)
#define A2DP_L2HC_SR_192000         (0x40)
#define A2DP_L2HC_SR_176400         (0x20)
#define A2DP_L2HC_SR_96000          (0x10)
#define A2DP_L2HC_SR_88200          (0x08)
#define A2DP_L2HC_SR_48000          (0x04)
#define A2DP_L2HC_SR_44100          (0x02)
#define A2DP_L2HC_SR_32000          (0x01)
#define A2DP_L2HC_SR_DATA(X)        (X & (A2DP_L2HC_SR_192000|A2DP_L2HC_SR_176400|A2DP_L2HC_SR_96000|A2DP_L2HC_SR_88200|A2DP_L2HC_SR_48000|A2DP_L2HC_SR_44100|A2DP_L2HC_SR_32000))

// Bitrate(kbps) MSB
#define A2DP_L2HC_BR_1920           (0x40)
#define A2DP_L2HC_BR_1600           (0x20)
#define A2DP_L2HC_BR_1280           (0x10)
#define A2DP_L2HC_BR_960            (0x08)
#define A2DP_L2HC_BR_640            (0x04)
#define A2DP_L2HC_BR_480            (0x02)
#define A2DP_L2HC_BR_320            (0x01)

// Bitrate(kbps) LSB
#define A2DP_L2HC_BR_256            (0x80)
#define A2DP_L2HC_BR_192            (0x40)
#define A2DP_L2HC_BR_128            (0x20)
#define A2DP_L2HC_BR_96             (0x10)
#define A2DP_L2HC_BR_64             (0x08)

#define A2DP_L2HC_BR_REMAP_1920     (0X8000)    /* b15:1920 kbps */
#define A2DP_L2HC_BR_REMAP_1600     (0X4000)    /* b14:1600 kbps */
#define A2DP_L2HC_BR_REMAP_1280     (0X2000)    /* b13:1280 kbps */
#define A2DP_L2HC_BR_REMAP_960      (0X1000)    /* b12:960 kbps */
#define A2DP_L2HC_BR_REMAP_640      (0X0800)    /* b11:640 kbps */
#define A2DP_L2HC_BR_REMAP_480      (0X0400)    /* b10:480 kbps */
#define A2DP_L2HC_BR_REMAP_320      (0X0200)    /* b9:320 kbps */
#define A2DP_L2HC_BR_REMAP_256      (0X0100)    /* b8:256 kbps */
#define A2DP_L2HC_BR_REMAP_192      (0X0080)    /* b7:192 kbps */
#define A2DP_L2HC_BR_REMAP_128      (0X0040)    /* b6:128 kbps */
#define A2DP_L2HC_BR_REMAP_96       (0X0020)    /* b5:96 kbps */
#define A2DP_L2HC_BR_REMAP_64       (0X0010)    /* b4:64 kbps */

// Frame Duration: Octet9;
#define A2DP_L2HC_FRAME_10MS        (0x02)
#define A2DP_L2HC_FRAME_7D5MS       (0x01)
#define A2DP_L2HC_FRAME_5MS         (0x80)
#define A2DP_L2HC_FD_MSB_DATA(X)    (X & (A2DP_L2HC_FRAME_10MS|A2DP_L2HC_FRAME_7D5MS))
#define A2DP_L2HC_FD_LSB_DATA(X)    (X & A2DP_L2HC_FRAME_5MS)

// Channel mode
#define A2DP_L2HC_CM_DUAL           (0X08)
#define A2DP_L2HC_CM_MONO           (0x04)
#define A2DP_L2HC_CM_DATA(X)        (X & (A2DP_L2HC_CM_DUAL|A2DP_L2HC_CM_MONO))

#define A2DP_L2HC_ALL_BITS_ZERO     (0x00)

#if defined(A2DP_L2HC_ON)
bt_status_t a2dp_codec_l2hc_init(int index);
void a2dp_codec_l2hc_common_init(void);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* __A2DP_CODEC_L2HC_H__ */