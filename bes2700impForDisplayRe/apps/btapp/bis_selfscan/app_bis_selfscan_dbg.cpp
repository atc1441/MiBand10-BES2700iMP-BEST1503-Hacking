/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifdef BIS_SELFSCAN_ENABLED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"
#include "app_trace_rx.h"

#include "bluetooth_ble_api.h"

#include "app_bis_selfscan.h"


/*********************external function declaration*************************/
int app_bis_selfscan_init(const app_bis_selfscan_init_cfg_t *p_init_cfg, const app_bis_selfscan_evt_cb_t *p_evt_cb);
void app_bis_selfscan_dp_rx_data_handler(const uint8_t *rx_data, uint16_t data_len);

/************************private macro defination***************************/

/************************private type defination****************************/
typedef void (*app_bis_selfscan_function_handle)(uint32_t BufPtr, uint32_t BufLen);
typedef struct
{
    const char *string;
    app_bis_selfscan_function_handle function;
} app_bis_selfscan_handle_t;

/**********************private function declaration*************************/

/************************private variable defination************************/
static void app_bis_selfscan_init(uint32_t BufPtr, uint32_t BufLen)
{
    // Store max 10
    // 5s scan timeout
    // 5s sync timeout
    app_bis_selfscan_init_cfg_t init_cfg = {10, 5, 5};
    app_bis_selfscan_init(&init_cfg, NULL);
}

static void app_bis_selfscan_dp_rx_from_uart_handler(uint8_t *p_data, uint16_t len)
{
    bt_thread_call_func_2(app_bis_selfscan_dp_rx_data_handler,
                                  bt_alloc_param_size(p_data, len),
                                  bt_fixed_param(len));
}

static void app_bis_selfscan_start_scan_cmd_handler(uint32_t BufPtr, uint32_t BufLen)
{
    uint8_t start_scan[] = {APP_BIS_SELFSCAN_OP_SCAN_COMMAND, APP_BIS_SELFSCAN_SCAN_START};
    app_bis_selfscan_dp_rx_from_uart_handler(start_scan, sizeof(start_scan));
}

static void app_bis_selfscan_stop_scan_cmd_handler(uint32_t BufPtr, uint32_t BufLen)
{
    uint8_t stop_scan[] = {APP_BIS_SELFSCAN_OP_SCAN_COMMAND, APP_BIS_SELFSCAN_SCAN_STOP};
    app_bis_selfscan_dp_rx_from_uart_handler(stop_scan, sizeof(stop_scan));
}

static void app_bis_selfscan_start_sync_cmd_handler(uint32_t BufPtr, uint32_t BufLen)
{
    if (BufPtr == 0 || BufLen == 0)
    {
        return;
    }

    uint8_t start_sync[2 + GAP_KEY_LEN] = {APP_BIS_SELFSCAN_OP_SRC_SELECT_CMD};
    uint8_t sync_lid = *(uint8_t *)BufPtr - '0';
    uint8_t *bcast_code = BufLen >= GAP_KEY_LEN + 1 ? (uint8_t *)BufPtr + 1 : NULL;

    start_sync[1] = sync_lid;
    if (bcast_code)
    {
        memcpy(start_sync + 2, bcast_code, GAP_KEY_LEN);
    }

    app_bis_selfscan_dp_rx_from_uart_handler(start_sync, sizeof(start_sync));
}

static void app_bis_selfscan_stop_sync_cmd_handler(uint32_t BufPtr, uint32_t BufLen)
{
    uint8_t stop_sync[] = {APP_BIS_SELFSCAN_OP_STOP_SYNC};
    app_bis_selfscan_dp_rx_from_uart_handler(stop_sync, sizeof(stop_sync));
}

static void app_bis_selfscan_set_bcast_code_cmd_handler(uint32_t BufPtr, uint32_t BufLen)
{
    uint8_t set_bcast_code[1 + GAP_KEY_LEN] = {APP_BIS_SELFSCAN_OP_BCAST_CODE};

    uint8_t *bcast_code = BufLen >= GAP_KEY_LEN ? (uint8_t *)BufPtr : NULL;

    if (bcast_code)
    {
        memcpy(set_bcast_code + 1, bcast_code, GAP_KEY_LEN);
    }

    app_bis_selfscan_dp_rx_from_uart_handler(set_bcast_code, sizeof(set_bcast_code));
}

static const app_bis_selfscan_handle_t pp_bis_selfscan_test_handle[] =
{
    {"init", app_bis_selfscan_init},
    {"scan_start", app_bis_selfscan_start_scan_cmd_handler},
    {"scan_stop", app_bis_selfscan_stop_scan_cmd_handler},
    {"sync_start", app_bis_selfscan_start_sync_cmd_handler},
    {"sync_stop", app_bis_selfscan_stop_sync_cmd_handler},
    {"bcast_code", app_bis_selfscan_set_bcast_code_cmd_handler},
};

int app_bis_selfscan_cmd_handler(unsigned char *buf, unsigned int length)
{
    int ret = 0;
    char *para_addr = NULL;
    unsigned int para_len = 0;

    for (uint32_t i = 0; i < ARRAY_SIZE(pp_bis_selfscan_test_handle); i++)
    {
        if ((strncmp((char *)buf, pp_bis_selfscan_test_handle[i].string, strlen(pp_bis_selfscan_test_handle[i].string)) == 0) ||
                strstr(pp_bis_selfscan_test_handle[i].string, (char *)buf))
        {
            para_addr = strstr((char *)buf, "|");
            if (para_addr != NULL)
            {
                para_addr++;
                para_len = length - (para_addr - (char *)buf);
            }

            pp_bis_selfscan_test_handle[i].function((uint32_t)para_addr, para_len);
            break;
        }

        if (i == ARRAY_SIZE(pp_bis_selfscan_test_handle))
        {
            ret = -1;
            TRACE(0, "ERROR:can not find handle function");
        }
    }

    return ret;
}

unsigned int app_bis_selfscan_cmd_callback(unsigned char *buf, unsigned int len)
{
    // Check len
    TRACE(1, "cmd: %s", buf);
    app_bis_selfscan_cmd_handler((unsigned char *)buf, strlen((char *)buf));
    return 0;
}

void app_bis_selfscan_cmd_init(void)
{
#ifdef APP_TRACE_RX_ENABLE
    app_trace_rx_register("BIS_SC", app_bis_selfscan_cmd_callback);
#endif
}

void app_bis_selfscan_cmd_deinit(void)
{
#ifdef APP_TRACE_RX_ENABLE
    app_trace_rx_deregister("BIS_SC");
#endif
}

#endif /* BIS_SELFSCAN_ENABLED */