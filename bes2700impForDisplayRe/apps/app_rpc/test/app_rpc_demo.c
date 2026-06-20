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
#include "cmsis.h"
#include "cmsis_os.h"
#include "plat_types.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "hal_mcu2sens.h"
#include "hal_sys2bth.h"
#include "app_rpc_api.h"
#include "app_rpc_demo.h"

static void rpc_test_timer_handler(void const *param);
osTimerDef(rpc_test_timer, rpc_test_timer_handler);
static osTimerId rpc_test_timer_id = NULL;

static void rpc_test_timer_handler(void const *param)
{
    char *str = "Hello! this is bth2dsp demo cmd!";
    app_rpc_send_cmd(APP_RPC_CORE_BTH_DSP, BTH_DSP_TASK_CMD_DEMO_REQ_NO_RSP, (uint8_t*)str, strlen(str) + 1);
}

void app_rpc_test_demo(void)
{
    TRACE(0, "%s", __func__);

    rpc_test_timer_id = osTimerCreate(osTimer(rpc_test_timer), osTimerPeriodic, NULL);
    if (rpc_test_timer_id != NULL) {
        osTimerStart(rpc_test_timer_id, 3000);
    }
}