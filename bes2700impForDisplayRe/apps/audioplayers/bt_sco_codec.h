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
#ifndef __BT_SCO_CODEC_H__
#define __BT_SCO_CODEC_H__

#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t (*bt_sco_codec_init_t)(uint32_t sample_rate);
typedef int32_t (*bt_sco_codec_deinit_t)(void);
typedef int32_t (*bt_sco_codec_encoder_t)(uint8_t *codec_buf, uint32_t codec_size, uint8_t *pcm_buf, uint32_t pcm_size);
typedef int32_t (*bt_sco_codec_decoder_t)(uint8_t *codec_buf, uint32_t codec_size, uint8_t *pcm_buf, uint32_t pcm_size);

typedef struct {
    bt_sco_codec_init_t     init;
    bt_sco_codec_deinit_t   deinit;
    bt_sco_codec_encoder_t  encoder;
    bt_sco_codec_decoder_t  decoder;
} bt_sco_codec_t;

extern bt_sco_codec_t bt_sco_codec_cvsd;
extern bt_sco_codec_t bt_sco_codec_msbc;
extern bt_sco_codec_t bt_sco_codec_xxxx;

#ifdef __cplusplus
}
#endif

#endif