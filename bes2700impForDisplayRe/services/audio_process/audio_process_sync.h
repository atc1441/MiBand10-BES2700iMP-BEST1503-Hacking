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
#ifndef __AUDIO_PROCESS_SYNC_H__
#define __AUDIO_PROCESS_SYNC_H__

#include "stdint.h"

typedef enum {
    AUDIO_PROCESS_SYNC_VOL,
    AUDIO_PROCESS_SYNC_QTY
} audio_process_sync_t;

#ifdef __cplusplus
extern "C" {
#endif

int32_t audio_process_sync_send(uint8_t *buf, uint32_t len);
int32_t audio_process_tws_sync_change(uint8_t *buf, uint32_t len);
#ifdef __cplusplus
}
#endif

#endif
