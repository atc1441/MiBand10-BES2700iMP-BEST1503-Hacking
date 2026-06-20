/***************************************************************************
 *
 * @copyright 2015-2023 BES.
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
 * @author wangzhuli
 * @date 2023.02.27         v1.0
 *
 ****************************************************************************/
#ifndef __APP_UI_IPSCAN_MGR_H__
#define __APP_UI_IPSCAN_MGR_H__

#define RADICAL_SAN_INTERVAL_NV_MASTER                  (BTIF_BT_DEFAULT_PAGE_SCAN_INTERVAL/4)
#define RADICAL_SAN_INTERVAL_NV_SLAVE                   ((BTIF_BT_DEFAULT_PAGE_SCAN_INTERVAL/4)-0x48)

#define RADICAL_SCAN_INTERVAL_BY_NV_ROLE                (app_ibrt_conn_is_nv_master() ? RADICAL_SAN_INTERVAL_NV_SLAVE : RADICAL_SAN_INTERVAL_NV_MASTER)

#define SCAN_INTERVAL_IN_SCO_TWS_DISCONNECTED           (BTIF_BT_DEFAULT_PAGE_SCAN_INTERVAL)
#define SCAN_WINDOW_IN_SCO_TWS_DISCONNECTED             (BTIF_BT_DEFAULT_PAGE_SCAN_WINDOW)
#define SCAN_INTERVAL_IN_SCO_TWS_CONNECTED              (BTIF_BT_DEFAULT_PAGE_SCAN_INTERVAL+0x400)
#define SCAN_WINDOW_IN_SCO_TWS_CONNECTED                (BTIF_BT_DEFAULT_PAGE_SCAN_WINDOW)
#define SCAN_INTERVAL_IN_A2DP_TWS_DISCONNECTED          (BTIF_BT_DEFAULT_PAGE_SCAN_INTERVAL)
#define SCAN_WINDOW_IN_A2DP_TWS_DISCONNECTED            (BTIF_BT_DEFAULT_PAGE_SCAN_WINDOW)
#define SCAN_INTERVAL_IN_A2DP_TWS_CONNECTED             (BTIF_BT_DEFAULT_PAGE_SCAN_INTERVAL+0x400)
#define SCAN_WINDOW_IN_A2DP_TWS_CONNECTED               (BTIF_BT_DEFAULT_PAGE_SCAN_WINDOW)

typedef enum scanUpdateEvent {
    SCAN_EV_ENABLE,
    SCAN_EV_DISABLE,
    SCAN_EV_ENTER_PAIRING,
    SCAN_EV_EXIT_PAIRING,
    SCAN_EV_A2DP_PLAYING,
    SCAN_EV_A2DP_SUSPEND,
    SCAN_EV_SCO_PLAYING,
    SCAN_EV_SCO_SUSPEND,

    SCAN_EV_UPDATE,

    SCAN_EV_ERROR,
} scanUpdateEvent;

void scanMgrInit();
void scanMgrReset();
void scanMgrUpdate(scanUpdateEvent event);
bool scanMgrModeStash();
void scanMgrModeRecover();
const char *scanEvt2Str(scanUpdateEvent event);

#endif /* #ifndef __APP_UI_IPSCAN_MGR_H__ */
