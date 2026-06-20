/***************************************************************************
 *
 * Copyright 2020-2035 BES.
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
#ifndef __APP_UI_PARAM_CONFIG_H__
#define __APP_UI_PARAM_CONFIG_H__

#define  IBRT_UI_CLOSE_BOX_EVENT_WAIT_RESPONSE_TIMEOUT              (600)//ms
#define  IBRT_UI_MOBILE_RECONNECT_WAIT_READY_TIMEOUT                (1000)//ms
#define  IBRT_UI_TWS_RECONNECT_WAIT_READY_TIMEOUT                   (1500)//ms
#define  IBRT_UI_RECONNECT_MOBILE_WAIT_RESPONSE_TIMEOUT             (5000)//5s
#define  IBRT_UI_RECONNECT_IBRT_WAIT_RESPONSE_TIMEOUT               (300)//ms
#define  IBRT_UI_NV_SLAVE_RECONNECT_TWS_WAIT_RESPONSE_TIMEOUT       (1000)//ms
#define  IBRT_UI_NV_MASTER_RECONNECT_TWS_WAIT_RESPONSE_TIMEOUT      (300)//ms
#define  IBRT_UI_DISABLE_BT_SCAN_TIMEOUT                            (150000)//5min

/// IBRT_UI_OPEN_RECONNECT_TWS_MAX_TIMES need max than IBRT_UI_DELAY_RECONN_MOBILE_MAX_TIMES
#define  IBRT_UI_DELAY_RECONN_MOBILE_MAX_TIMES              (3)

#define  IBRT_UI_OPEN_RECONNECT_MOBILE_MAX_TIMES            (3)
#ifdef PRODUCTION_LINE_PROJECT_ENABLED
#define  IBRT_UI_OPEN_RECONNECT_TWS_MAX_TIMES               (0xFFFF)
#define  IBRT_UI_RECONNECT_TWS_MAX_TIMES                    (0xFFFF)
#else
#define  IBRT_UI_OPEN_RECONNECT_TWS_MAX_TIMES               (6)
#define  IBRT_UI_RECONNECT_TWS_MAX_TIMES                    (20)
#endif

#define  IBRT_UI_RX_SEQ_ERROR_TIMEOUT                     (10000)
#define  IBRT_UI_RX_SEQ_ERROR_THRESHOLD                   (50)
#define  IBRT_UI_RX_SEQ_ERROR_RECOVER_TIMEOUT             (5000)

#define  IBRT_TWS_SWITCH_RSSI_THRESHOLD                   (30)
#define  IBRT_UI_RADICAL_SAN_INTERVAL_NV_MASTER           ((BTIF_BT_DEFAULT_PAGE_SCAN_INTERVAL/4)-0x24)
#define  IBRT_UI_RADICAL_SAN_INTERVAL_NV_SLAVE            ((BTIF_BT_DEFAULT_PAGE_SCAN_INTERVAL/4)-0x48)

#endif

