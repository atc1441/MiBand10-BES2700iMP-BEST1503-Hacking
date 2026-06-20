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
#include <stdbool.h>
#include <stdint.h>

#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "customparam_section.h"
#include "gsound_target.h"
#include "gsound_custom_reset.h"

#include "gsound_custom_bt.h"

//
// Target Reset functions
//
void GSoundTargetGetCrashDump(void (*callback)(const GSoundCrashFatalDump *gsound_crash))
{
    GSoundCrashFatalDump gsound_crash;
    memset(&gsound_crash, 0, sizeof(gsound_crash));
    gsound_crash_convert(&gsound_crash, gsound_get_plateform_reset());
    callback(&gsound_crash);
}

GSoundStatus GSoundTargetGetResetReason(GSoundResetReason *gsound_reset)
{
    gsound_reset_convert(gsound_reset,
                         gsound_get_plateform_reset());

    return GSOUND_STATUS_OK;
}

//
// Target time functions.
//
uint32_t GSoundTargetOsGetTicksMs(void)
{
    return GET_CURRENT_MS();
}
