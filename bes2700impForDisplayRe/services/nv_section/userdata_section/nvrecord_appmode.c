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
#if defined(NEW_NV_RECORD_ENABLED)

#include "hal_trace.h"
#include "nvrecord_appmode.h"

#define nvrec_trace TRACE


nvrec_appmode_e nv_record_appmode_get(void)
{
    NV_RECORD_APP_MODE_INFO_T *pModeInfo =
        (NV_RECORD_APP_MODE_INFO_T *)(&(nvrecord_extension_p->app_mode_info));

    nvrec_trace(0, "NV_APPMODE: %d", pModeInfo->app_mode);

    return pModeInfo->app_mode;
}

void nv_record_appmode_set(nvrec_appmode_e mode)
{
    NV_RECORD_APP_MODE_INFO_T *pModeInfo =
        (NV_RECORD_APP_MODE_INFO_T *)(&(nvrecord_extension_p->app_mode_info));
#ifndef RAM_NV_RECORD
    uint32_t lock = nv_record_pre_write_operation();
    if (pModeInfo->app_mode != mode)
    {
        nv_record_update_runtime_userdata();
    }
    pModeInfo->app_mode = mode;
    nv_record_post_write_operation(lock);
#else
    pModeInfo->app_mode = mode;
#endif
}

#endif
