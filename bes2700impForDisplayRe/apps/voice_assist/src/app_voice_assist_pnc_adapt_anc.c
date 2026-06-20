
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
#include "hal_trace.h"
#include "app_anc_assist.h"
#include "anc_assist.h"
#include "app_voice_assist_pnc_adapt_anc.h"
 static int32_t _voice_assist_pnc_adapt_anc_callback(void *buf, uint32_t len, void *other);
 int32_t app_voice_assist_pnc_adapt_anc_init(void)
 {
     app_anc_assist_register(ANC_ASSIST_USER_PNC_ADAPT_ANC, _voice_assist_pnc_adapt_anc_callback);
     return 0;
 }
int32_t app_voice_assist_pnc_adapt_anc_open(void)
{
    TRACE(0, "[%s] pnc adapt anc start stream", __func__);
    app_anc_assist_open(ANC_ASSIST_USER_PNC_ADAPT_ANC);
    return 0;
}
int32_t app_voice_assist_pnc_adapt_anc_close(void)
{
    TRACE(0, "[%s] pnc adapt anc close stream", __func__);
    app_anc_assist_close(ANC_ASSIST_USER_PNC_ADAPT_ANC);
    return 0;
}
static int32_t _voice_assist_pnc_adapt_anc_callback(void * buf, uint32_t len, void *other)
{
    return 0;
}
