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
#ifdef MCU2SENS_MSG_TEST

#include "plat_types.h"
#include "sens_comp_test.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "sensor_hub.h"
#include "sensor_hub_core.h"
#include "string.h"

#define MAX_TX_CNT                          10
static uint8_t tx_buf[MAX_TX_CNT][8];
static uint32_t tx_cnt;

static unsigned int msg_test_rx_handler(const void *data, unsigned int len)
{
    int ret;
    uint32_t tx_len;
    const uint8_t *rdata = data;

    TR_INFO(0, "RX=%u", len);
    DUMP8("%02X ", data, len);

    if (len > sizeof(tx_buf[0])) {
        tx_len = sizeof(tx_buf[0]);
    } else {
        tx_len = len;
    }

    for (uint32_t i = 0; i < tx_len; i++) {
        tx_buf[tx_cnt][i] = rdata[i] + 1;
    }
    ret = sensor_hub_send(tx_buf[tx_cnt], tx_len);
    ASSERT(ret == 0, "hal_mcu2sens_send failed: %d", ret);

    tx_cnt++;
    if (tx_cnt >= MAX_TX_CNT) {
        tx_cnt = 0;
    }

    return len;
}

static void msg_test_tx_handler(const void *data, unsigned int len)
{
    TR_INFO(0, "TX=%u", len);
    DUMP8("%02X ", data, len);
}

void mcu2sens_msg_test(void)
{
    int ret;
    uint32_t tx_len;
    const char data[] = "gogo";

    TR_INFO(0, "%s", __func__);
    sensor_hub_core_register_rx_irq_handler(msg_test_rx_handler);
    sensor_hub_core_register_tx_done_irq_handler(msg_test_tx_handler);

    hal_sys_timer_delay(MS_TO_TICKS(1));

    tx_len = sizeof(data) - 1;
    if (tx_len > ARRAY_SIZE(tx_buf)) {
        tx_len = ARRAY_SIZE(tx_buf);
    }
    memcpy(tx_buf, data, tx_len);
    ret = sensor_hub_send(tx_buf, tx_len);
    ASSERT(ret == 0, "hal_mcu2sens_send failed: %d", ret);
}

#endif
