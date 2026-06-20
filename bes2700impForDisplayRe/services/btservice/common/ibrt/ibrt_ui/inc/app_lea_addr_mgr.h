/***************************************************************************
 *
 * Copyright 2024-2034 BES.
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

#ifndef __APP_LEA_ADDR_MGR_H__
#define __APP_LEA_ADDR_MGR_H__
#include "stdint.h"

#define APP_LEA_ADDR_MGR_IRK_LEN      (16)

typedef struct
{
    uint8_t                    cmd_type;
    uint8_t                    nv_role;
    uint8_t                    current_role;
    bool                       tws_is_connected;
    bool                       le_connection_exist;
    uint8_t                    connected_lea_count;
    bt_bdaddr_t                used_addr;
    bt_bdaddr_t                master_addr;
    bt_bdaddr_t                slave_addr;
    uint8_t                    master_irk[APP_LEA_ADDR_MGR_IRK_LEN];
    uint8_t                    slave_irk[APP_LEA_ADDR_MGR_IRK_LEN];
} app_lea_addr_mgr_ctx_t;

#ifdef __cplusplus
extern "C" {
#endif

void app_lea_addr_recv_exchange_handle(app_lea_addr_mgr_ctx_t *dev_mgr, uint16_t rsp_seq);

void app_lea_addr_recv_exchange_rsp_handle(app_lea_addr_mgr_ctx_t *dev_mgr);

void app_lea_addr_recv_exchange_rsp_timeout_handle(app_lea_addr_mgr_ctx_t *dev_mgr);

void app_lea_addr_mgr_handle_dev_connection_evt(app_ui_evt_t event);

void app_lea_addr_mgr_init(void);

#ifdef __cplusplus
}
#endif

#endif //__APP_LEA_ADDR_MGR_H__
