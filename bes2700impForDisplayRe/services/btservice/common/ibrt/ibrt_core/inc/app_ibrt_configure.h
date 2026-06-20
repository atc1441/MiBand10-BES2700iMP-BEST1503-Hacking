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
#ifndef __APP_IBRT_CONFIGURE_H__
#define __APP_IBRT_CONFIGURE_H__

#define  IBRT_UI_ROLE_SWITCH_TIME_THRESHOLD                         (2)  //
#define  IBRT_UI_ROLE_SWITCH_THRESHOLD_WITH_RSSI                    (10) //dbm

#define  IBRT_UI_RECONNECT_MOBILE_MAX_TIMES                 (20)
#define  IBRT_UI_RECONNECT_IBRT_MAX_TIMES                   (20)

#define  IBRT_MOBILE_PAGE_TIMEOUT                           (0x2000)
#define  IBRT_TWS_PAGE_TIMEOUT                              (0x2800)

#define  IBRT_UI_TWS_CONNECTION_TIMEOUT                   (3200)//1600 slot=1s/3200 slot = 2s/8000slot=5s

#define  IBRT_STOP_IBRT_WAIT_TIME                         (500)
#define  TWS_CONN_FAILED_WAIT_TIME                        (5)

/*
* ibrt ui event error handling timeout
*/
#define  CONNECT_NO_03_TIMEOUT                            (18000)
#define  DISCONNECT_NO_05_TIMEOUT                         (8000)

#define  IBRT_UI_TWS_CMD_SEND_TIMEOUT                     (10000)
#define  IBRT_UI_TWS_COUNTER_THRESHOLD                    (500000)
#define  IBRT_UI_TWS_SWITCH_STABLE_TIMEOUT                (300)
#ifdef TEST_OVER_THE_AIR_ENANBLED
#define  IBRT_UI_RSSI_MONITOR_TIMEOUT                     (1000)//ms
#else
#define  IBRT_UI_RSSI_MONITOR_TIMEOUT                     (5000)//ms
#endif

#endif

