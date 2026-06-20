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

#ifndef __BLE_AUDIO_MOBILE_STM_H__
#define __BLE_AUDIO_MOBILE_STM_H__

#include "hsm.h"
#include "ble_audio_define.h"

typedef enum
{
    REQ_MOBILE_BLE_CONNECT,
    REQ_CANCEL_BLE_ACTION,
    REQ_MOBILE_BLE_DISCONNECT,

    EVT_MOBILE_CONNECTING_CANCELLED,
    EVT_MOBILE_BLE_CONNECTED,
    EVT_MOBILE_BOND_SUCCESS,
    EVT_MOBILE_BOND_FAILURE,
    EVT_MOBILE_ENCRYPT,
    EVT_MOBILE_BLE_CONNECT_TIMEOUT,
    EVT_MOBILE_BLE_DISCONNECTED,
}BLE_MOBILE_CONN_EVENT;

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
} ble_mobile_stm_t;

void app_ble_mobile_sm_timer_init(ble_mobile_stm_t *mobile_device);

void ble_mobile_sm_init(ble_mobile_stm_t *ble_mobile_sm);

void ble_mobile_sm_startup(ble_mobile_stm_t *ble_mobile_sm);

bool ble_audio_mobile_link_disconnected(ble_mobile_stm_t *ble_mobile_sm);

void ble_audio_mobile_link_send_message(ble_mobile_stm_t *mobile_instance,BLE_MOBILE_CONN_EVENT event, uint32_t param0, uint32_t param1);

bool ble_audio_mobile_link_is_connected(ble_mobile_stm_t *ble_mobile_sm);

#endif
