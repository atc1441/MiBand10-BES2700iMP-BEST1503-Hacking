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
#ifndef __VOICE_TX_AEC_H__
#define __VOICE_TX_AEC_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    VOICE_TX_AEC_SET_BYPASS_DISABLE = 0,
    VOICE_TX_AEC_SET_BYPASS_ENABLE,
} voice_tx_aec_ctl_t;

int32_t voice_tx_aec_open(uint32_t frame_len, uint32_t bits);

int32_t voice_tx_aec_close(void);

// Get/set some value or enable/disable some function
int32_t voice_tx_aec_ctl(voice_tx_aec_ctl_t ctl, void *ptr);

// Process voice stream
int32_t voice_tx_aec_process(int16_t *pcm_buf, int16_t *ref_buf, int32_t pcm_len, int16_t *out_buf);

// Dump some parameters
int32_t voice_tx_aec_dump(void);

#ifdef __cplusplus
}
#endif

#endif
