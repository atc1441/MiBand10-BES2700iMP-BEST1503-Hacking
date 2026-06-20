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
#ifndef __APP_ANC_ASSIST_TRIGGER_H__
#define __APP_ANC_ASSIST_TRIGGER_H__

#include "hal_aud.h"

#define MAX_STREAM_NUM (2)

typedef struct
{
    enum AUD_STREAM_USE_DEVICE_T device;
    enum AUD_STREAM_T stream;
} app_anc_assist_trigger_stream_t;

typedef struct
{
    app_anc_assist_trigger_stream_t stream[MAX_STREAM_NUM];
    uint8_t stream_num;
    uint8_t trigger_channel;
} app_anc_assist_trigger_device_t;

#ifdef __cplusplus
extern "C" {
#endif

int app_anc_assist_trigger_checker(void);
int app_anc_assist_trigger_init(void);
int app_anc_assist_trigger_deinit(void);

#ifdef __cplusplus
}
#endif

#endif
