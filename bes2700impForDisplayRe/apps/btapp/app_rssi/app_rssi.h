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
#ifndef __APP_RSSI_H__
#define __APP_RSSI_H__
#include <stdint.h>
#include "bt_common_define.h"

#define APP_RSSI_WINDOW_SIZE            (15)
#define APP_RSSI_MAX_RECORD_NUM         (2)

#define APP_RSSI_BASE_SAMPLE_INTERVAL   (50)
#define APP_RSSI_TWS_SAMPLE_INTERVAL    (200)

#define APP_RSSI_GET_PEER_RSSI_INTERVAL (3000)
#define APP_RSSI_SAMPLE_TIMEOUT         (15000)


#define APP_RSSI_ALWAYS_TRIGGER         (0)
#define APP_RSSI_EVENT_TRIGGER          (1)

#define APP_RSSI_TRIGGER_MODE           APP_RSSI_EVENT_TRIGGER

typedef struct
{
    int8_t rssi;
    int8_t rssi_min;
    int8_t rssi_max;
    uint8_t rxgain;
    uint8_t ser;
} rssi_t;

typedef struct
{
    /* remote addr */
    bt_bdaddr_t addr;

    /* local rssi */
    rssi_t local_rssi;

    /* !record peer rssi if snoop connect */
    rssi_t peer_rssi;
} remote_rssi_t;

typedef struct
{
    /* tws side */
    uint8_t side;

    /* tws rssi */
    rssi_t tws_rssi;

    /* remote device num */
    uint8_t remote_num;
    remote_rssi_t remote_rssi[APP_RSSI_MAX_RECORD_NUM];
} rssi_pkt_t;

void app_rssi_init();
void app_rssi_update_date_pkt(rssi_pkt_t *p_pkt);
void app_rssi_sample_task_start();
void app_rssi_sample_task_stop();

#endif /*__APP_RSSI_H__ */
