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
#ifdef SENS_TRC_TO_MCU

#include "plat_types.h"
#include "hal_mcu2sens.h"
#include "hal_trace.h"
#include "rmt_trace_server.h"
#include "string.h"

static struct RMT_TRC_SERVER_CFG_T server_cfg;
static const char *trc_name = "SENS";

static int sens_trace_send(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_mcu2sens_send_seq(HAL_MCU2SENS_ID_1, data, len, seq);
}

static void sens_trace_server_dump(void)
{
    rmt_trace_server_dump(&server_cfg);
}

static unsigned int msg_test_rx_handler(const void *data, unsigned int len)
{
    return rmt_trace_server_msg_handler(&server_cfg, data, len);
}

void sensor_hub_trace_server_open(void)
{
    int ret;

    memset(&server_cfg, 0, sizeof(server_cfg));
    server_cfg.name = trc_name;
    server_cfg.first_msg = true;
    server_cfg.send_cb = sens_trace_send;

    ret = hal_mcu2sens_open(HAL_MCU2SENS_ID_1, msg_test_rx_handler, NULL, false);
    ASSERT(ret == 0, "hal_mcu2sens_open failed: %d", ret);

    ret = hal_mcu2sens_start_recv(HAL_MCU2SENS_ID_1);
    ASSERT(ret == 0, "hal_mcu2sens_start_recv failed: %d", ret);

    rmt_trace_server_dump_register(sens_trace_server_dump);

    TRACE(0, "Start to rx sensor_hub trace");
}

void sensor_hub_trace_server_close(void)
{
    rmt_trace_server_dump_deregister(sens_trace_server_dump);
    hal_mcu2sens_stop_recv(HAL_MCU2SENS_ID_1);
    hal_mcu2sens_close(HAL_MCU2SENS_ID_1);

    TRACE(0, "Stop rx sensor_hub trace");
}

#endif
