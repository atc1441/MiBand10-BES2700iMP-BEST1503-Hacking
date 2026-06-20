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
#ifndef __VOICE_DEV_DEFS_H__
#define __VOICE_DEV_DEFS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LOG_INFO            TRACE
#define LOG_WARNING         TRACE

#define FRAME_MS_TO_LEN(fs, ms)             ((ms) * (fs) / 1000)
#define FRAME_LEN_TO_MS(fs, len)            ((len) / ( (fs) / 1000))
#define SAMPLE_BITS_TO_BYTES(bits)          ((bits) / 8)
#define SAMPLE_BYTES_TO_BITS(bytes)         ((bytes) * 8)


// TODO: Add CHANNEL_NUM
// NOTE: SAMPLE_BITS: 16 or 32
#define SAMPLE_BITS_MAX         (16)
#define SAMPLE_RATE_MAX         (16000)
#define FRAME_MS_MAX            (16)
#define FRAME_LEN_MAX           FRAME_MS_TO_LEN(SAMPLE_RATE_MAX, FRAME_MS_MAX)
#define FRAME_SIZE_MAX          (FRAME_LEN_MAX * SAMPLE_BITS_TO_BYTES(SAMPLE_BITS_MAX))


#define SAMPLE_BITS             (16)
#define SAMPLE_RATE             (16000)
#define FRAME_MS                (16)
#define FRAME_LEN               FRAME_MS_TO_LEN(SAMPLE_RATE, FRAME_MS)
#define FRAME_SIZE              (FRAME_LEN_MAX * SAMPLE_BITS_TO_BYTES(SAMPLE_BITS))

#ifdef __cplusplus
}
#endif

#endif
