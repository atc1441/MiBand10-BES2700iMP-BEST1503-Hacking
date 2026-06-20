
/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifdef EVR_TRACING

#include "hal_evr.h"
#include "SEGGER_SYSVIEW.h"


int hal_evr_init(int evr_tracer)
{
    SEGGER_SYSVIEW_Conf();
    SEGGER_SYSVIEW_Start();

    return 0;
}

#endif /*EVR_TRACING*/
