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
#ifndef __BT_SCO_CHAIN_REFRAME_H__
#define __BT_SCO_CHAIN_REFRAME_H__

#include "plat_types.h"
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    REFRAME_CHANNEL_FF = 0,         // ANC FF MIC
    REFRAME_CHANNEL_FB,             // ANC FB MIC
    REFRAME_CHANNEL_VP,             // Voice Pick MIC
    // REFRAME_CHANNEL_EV,             // Environment MIC 
    REFRAME_CHANNEL_REF,            // ECHO REF
    // REFRAME_CHANNEL_BS,             // Bone Sensor
    REFRAME_CHANNEL_QTY
} reframe_channel_t;

int32_t bt_sco_chain_reframe_open(uint32_t tx_sample_rate, uint32_t tx_frame_len);
int32_t bt_sco_chain_reframe_close(void);

int32_t bt_sco_chain_reframe_cache_tx_pcm(void *pcm_buf, uint32_t frame_len, reframe_channel_t ch);
int32_t bt_sco_chain_reframe_tx_process(void *in_buf[], uint32_t frame_len, void *out_buf);

#ifdef __cplusplus
}
#endif

#endif
