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

#ifndef __BT_SVC_LEA_UC_CLIENT_CONN_STM_H__
#define __BT_SVC_LEA_UC_CLIENT_CONN_STM_H__
#include "hsm.h"

typedef struct
{
    Hsm     super;
    State   connecting;
    State   connected;
    State   disconnecting;
      State cancel_disconecting;
    State   disconnected;

    uint8_t         link_id;
    osTimerId       ble_mobile_link_moniter_timer_id;
    osTimerDefEx_t  ble_mobile_link_moniter_timer_def;
    osTimerId       ble_streming_check_timer_id;
    osTimerDefEx_t  ble_streming_chec_timer_def;
    bool            used;
    uint8_t         conidx;
    ble_bdaddr_t    address;
    uint8_t         disconn_reson;

    uint8_t public_addr[BTIF_BD_ADDR_SIZE];
    bool entry_connected;
}  bt_svc_lea_uc_server_stm_t;

void  bt_svc_lea_uc_server_sm_timer_init( bt_svc_lea_uc_server_stm_t *mobile_device);

void  bt_svc_lea_uc_server_sm_init( bt_svc_lea_uc_server_stm_t *ble_mobile_sm);

void  bt_svc_lea_uc_server_sm_startup( bt_svc_lea_uc_server_stm_t *ble_mobile_sm);

bool  bt_svc_lea_uc_server_link_disconnected( bt_svc_lea_uc_server_stm_t *ble_mobile_sm);

void  bt_svc_lea_uc_server_link_send_message( bt_svc_lea_uc_server_stm_t *mobile_instance,BLE_MOBILE_CONN_EVENT event, uint32_t param0, uint32_t param1);

bool  bt_svc_lea_uc_server_link_is_connected( bt_svc_lea_uc_server_stm_t *ble_mobile_sm);

bt_svc_lea_uc_server_stm_t* bt_svc_lea_uc_server_find_ble_mobile_sm_by_address(ble_bdaddr_t *addr);

bt_svc_lea_uc_server_stm_t* bt_svc_lea_uc_server_find_ble_mobile_sm_by_conidx(uint8_t conidx);

bt_svc_lea_uc_server_stm_t* bt_svc_lea_uc_server_make_ble_mobile_sm_by_address(ble_bdaddr_t *addr);

#endif
