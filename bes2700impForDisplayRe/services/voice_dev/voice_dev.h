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
#ifndef __VOICE_DEV_H__
#define __VOICE_DEV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "hal_aud.h"
#if 0
#include "vendor.h"
#endif

typedef enum {
    VOICE_DEV_SET_SUPPORT_ALGOS = 0,
    VOICE_DEV_SET_USER_ALGOS,
    VOICE_DEV_GET_ALGO,
    VOICE_DEV_SET_ALGO_FRAME_LEN,

    VOICE_DEV_CTL_QTY
} voice_dev_ctl_t;

typedef enum {
    VOICE_DEV_USER0 = 0,        // cfg voice dev
    VOICE_DEV_USER1,            // Gsound
    VOICE_DEV_USER2,            // Noise Estimation
    VOICE_DEV_USER3,
    VOICE_DEV_USER4,
    VOICE_DEV_USER5,
    VOICE_DEV_USER6,
    VOICE_DEV_USER7,
    VOICE_DEV_USER8,            // Test

    VOICE_DEV_USER_QTY
} voice_dev_user_t;

typedef enum {
    VOICE_DEV_BUF_INDEX_MIC0 = 0,
    VOICE_DEV_BUF_INDEX_MIC1,
    VOICE_DEV_BUF_INDEX_REF0,
    VOICE_DEV_BUF_INDEX_REF1,
    VOICE_DEV_BUF_INDEX_AEC,
    VOICE_DEV_BUF_INDEX_NOISE_EST,

    VOICE_DEV_OUT_BUF_PTR_NUM
} voice_dev_buf_index_t;

// TODO: Format: index, map and string
typedef enum {
    VOICE_DEV_ALGO_INDEX_DC_FILTER = 0,
    VOICE_DEV_ALGO_INDEX_PRE_GAIN,
    VOICE_DEV_ALGO_INDEX_AEC,
    VOICE_DEV_ALGO_INDEX_NOISE_EST,
    VOICE_DEV_ALGO_INDEX_POST_GAIN,
    VOICE_DEV_ALGO_INDEX_END
} voice_dev_algo_index_t;

typedef enum {
    VOICE_DEV_ALGO_NONE = 0,
    VOICE_DEV_ALGO_DC_FILTER = 0x01 << VOICE_DEV_ALGO_INDEX_DC_FILTER,
    VOICE_DEV_ALGO_PRE_GAIN = 0x01 << VOICE_DEV_ALGO_INDEX_PRE_GAIN,
    VOICE_DEV_ALGO_AEC = 0x01 << VOICE_DEV_ALGO_INDEX_AEC,
    VOICE_DEV_ALGO_NOISE_EST = 0x01 << VOICE_DEV_ALGO_INDEX_NOISE_EST,
    VOICE_DEV_ALGO_POST_GAIN = 0x01 << VOICE_DEV_ALGO_INDEX_POST_GAIN,
} voice_dev_algo_t;

typedef uint32_t (*callback_handler_t)(uint8_t *buf_ptr[], uint32_t frame_len);

typedef struct {
    uint32_t adc_bits;                  // Support: 16 and 32
    uint32_t frame_ms;
    uint32_t non_interleave;            // 0 or 1
    uint32_t algos;
    callback_handler_t handler;
} voice_dev_cfg_t;

// Call this function when platform is initialization
int32_t voice_dev_init(void);

int32_t voice_dev_open(voice_dev_user_t user, const voice_dev_cfg_t *cfg);
int32_t voice_dev_close(voice_dev_user_t user);

int32_t voice_dev_start(voice_dev_user_t user);
int32_t voice_dev_stop(voice_dev_user_t user);

int32_t voice_dev_ctl(voice_dev_user_t user, voice_dev_ctl_t ctl, void *ptr);


// voice_dev test
void test_voice_dev_open(void);
void test_voice_dev_start(void);
void test_voice_dev_stop(void);
void test_voice_dev_close(void);


#ifdef __cplusplus
}
#endif

#endif
