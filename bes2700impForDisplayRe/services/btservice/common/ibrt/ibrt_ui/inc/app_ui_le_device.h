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

#ifndef __APP_UI_LEAUDIO_DEVICE_H__
#define __APP_UI_LEAUDIO_DEVICE_H__

#include <stdint.h>

typedef struct
{
    /// BD Address of device
    uint8_t addr[BTIF_BD_ADDR_SIZE];
    /// Address type of the device 0=public/1=private random
    uint8_t addr_type;
} ble_addr_t;

typedef enum
{
    LE_DISCONNECTED,
    LE_CONNECTED,
    LE_DISCONNECTING
} LE_DEVICE_SM_STATE_E;

typedef struct
{
    bool used;
    bool destroying;

    LE_DEVICE_SM_STATE_E state;
    app_ui_evt_queue_t evt_que;
    app_ui_evt_t       active_evt;
    app_ui_evt_t       latest_evt;

    bool run_complete;

    uint8_t                 id;
    uint8_t             conidx;
    ble_addr_t         address;
    bool          bond_success;
    bool       using_ble_audio;

} le_device_sm;

void le_device_sm_init(uint8_t id, le_device_sm *device);
void le_device_sm_start(le_device_sm *device);
bool le_device_sm_on_evt(le_device_sm *device, uint32_t evt, uint32_t param0, uint32_t param1);

void le_device_set_active_evt(le_device_sm *me, app_ui_evt_t evt);

bool le_device_is_run_complete(le_device_sm *me);
bool le_device_is_running(le_device_sm *me);
bool le_device_is_disconnected(le_device_sm *me);
bool le_device_using_audio(le_device_sm *me);
void le_device_pop_run_complete_info(le_device_sm *me, app_ui_evt_t *active, app_ui_evt_t *latest);
void le_device_destroy(le_device_sm *device);


#endif
