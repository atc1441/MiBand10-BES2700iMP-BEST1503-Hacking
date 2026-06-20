/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#if !(defined(CHIP_SUBSYS_SENS) || (defined(CHIP_SUBSYS_BTH) ^ defined(BTH_AS_MAIN_MCU)))
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "pmu.h"
#include "dsp_m55.h"
#include "mcu_dsp_m55_app.h"

static bool app_dsp_m55_inited = false;

void app_core_bridge_data_tx_done(const void* data, unsigned int len)
{

}

unsigned int app_core_bridge_data_received(const void* data, unsigned int len)
{
    return len;
}

// mcu to initialize sensor hub module
void app_dsp_m55_init(void)
{
    if (app_dsp_m55_inited) {
        return;
    }
    //app_core_bridge_init();
    pmu_1803_uart_enable();
    int ret = dsp_m55_open(app_core_bridge_data_received, app_core_bridge_data_tx_done);
    ASSERT(ret == 0, "dsp_m55_open failed: %d", ret);

    app_dsp_m55_inited = true;
}

void app_dsp_m55_deinit(void)
{
    if (!app_dsp_m55_inited) {
        return;
    }

    dsp_m55_close();
    osDelay(10);

    // free all of the tx mailbox
    //app_core_bridge_free_tx_mailbox();

    // release the waiting tx done semphore
    //app_core_bridge_data_tx_done(NULL, 0);
    app_dsp_m55_inited = false;
}

#endif
