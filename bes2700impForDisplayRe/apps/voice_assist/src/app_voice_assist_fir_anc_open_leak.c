
/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifdef VOICE_ASSIST_FF_FIR_LMS
#include "hal_trace.h"
#include "app_anc_assist.h"
#include "anc_assist.h"
#include "app_voice_assist_fir_anc_open_leak.h"
#include "app_voice_assist_fir_lms.h"
 static int32_t _voice_assist_fir_anc_open_leak_callback(void *buf, uint32_t len, void *other);
 int32_t app_voice_assist_fir_anc_open_leak_init(void)
 {
    app_anc_assist_register(ANC_ASSIST_USER_FIR_ANC_OPEN_LEAK, _voice_assist_fir_anc_open_leak_callback);
    return 0;
 }
int32_t app_voice_assist_fir_anc_open_leak_open(void)
{
    TRACE(0, "[%s] fir anc open leak start stream", __func__);
    app_anc_assist_open(ANC_ASSIST_USER_FIR_ANC_OPEN_LEAK);
    return 0;
}
int32_t app_voice_assist_fir_anc_open_leak_close(void)
{
    TRACE(0, "[%s] fir anc open leak close stream", __func__);
    app_anc_assist_close(ANC_ASSIST_USER_FIR_ANC_OPEN_LEAK);
    return 0;
}
static int32_t _voice_assist_fir_anc_open_leak_callback(void * buf, uint32_t len, void *other)
{
    uint32_t *res = (uint32_t *)buf;
    uint32_t wear_leak_status = *res;
    TRACE(0, "[%s] wear_leak_status = %d", __func__, wear_leak_status);
    if (wear_leak_status == 1) {
        // app_voice_assist_fir_lms_open();
    } else {
        TRACE(1, "1111111111111111111111111111");
    }
    return 0;
}
#endif
