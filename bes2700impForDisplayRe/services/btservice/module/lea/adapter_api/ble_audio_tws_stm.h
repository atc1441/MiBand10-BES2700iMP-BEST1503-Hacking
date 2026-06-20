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

#ifndef __BLE_AUDIO_TWS_STM_H__
#define __BLE_AUDIO_TWS_STM_H__

#include "ble_audio_define.h"

typedef enum
{
    BLE_REQ_TWS_CONNECT,
    BLE_REQ_TWS_CANCEL_CONNECTION,
    BLE_REQ_TWS_DISCONNECT,

    BLE_EVT_TWS_CONNECTED,
    BLE_EVT_TWS_DISCONNECTED,
    BLE_EVT_TWS_CANCELED_COMPLETE,

    BLE_EVT_TWS_CONNECTION_FAILED,
    BLE_EVT_TWS_SM_TIMEOUT,
} BLE_TWS_EVENT_E;

void ble_audio_tws_stm_send_msg(BLE_TWS_EVENT_E event, uint32_t para0, uint32_t para1, uint32_t para2);

void ble_audio_tws_stm_init(void);

bool ble_audio_tws_stm_check_is_peer_addr(uint8_t *addr);


#endif