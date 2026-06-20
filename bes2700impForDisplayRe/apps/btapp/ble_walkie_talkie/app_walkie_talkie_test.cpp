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
#include "hal_trace.h"
#include "app_trace_rx.h"
#include <string.h>
#include "app_walkie_talkie_test.h"
#include "walkie_talkie_test.h"
#include "walkie_talkie.h"
#include "app_walkie_talkie.h"
#include "walkie_talkie_dbg.h"


static void app_wt_entry(uint8_t cmd_len, uint8_t *cmd_param)
{
    LOG_I("%s",__func__);
    app_walkie_talkie_init();
}

static void app_wt_exit(uint8_t cmd_len, uint8_t *cmd_param)
{
    LOG_I("%s",__func__);
    app_walkie_talkie_exit();
}

static const walkie_talkie_uart_handle_t app_wt_uart_test_handle[] =
{
    {"wt_entry",app_wt_entry},
    {"wt_exit", app_wt_exit},
};

unsigned int app_wt_uart_cmd_callback(unsigned char *cmd, unsigned int cmd_length)
{
    POSSIBLY_UNUSED int param_len = 0;
    char* cmd_param = NULL;
    char* cmd_end = (char *)cmd + cmd_length;

    cmd_param = strstr((char*)cmd, (char*)"|");

    if (cmd_param)
    {
        *cmd_param = '\0';
        cmd_length = cmd_param - (char *)cmd;
        cmd_param += 1;

        param_len = cmd_end - cmd_param;
    }

    wt_test_uart_cmd_handler((unsigned char*)cmd,
            strlen((char*)cmd),app_wt_uart_test_handle,ARRAY_SIZE(app_wt_uart_test_handle));

    return 0;
}

void app_wt_uart_cmd_register(void)
{
#ifndef WT_GAP_UNIT_TEST
        return;
#endif

#ifdef APP_TRACE_RX_ENABLE
    TRACE(0,"%s",__func__);
    app_trace_rx_register("APP_WT", app_wt_uart_cmd_callback);
#endif
}

