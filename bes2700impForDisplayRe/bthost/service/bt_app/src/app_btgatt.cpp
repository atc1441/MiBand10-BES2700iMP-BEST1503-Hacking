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
#if defined(__GATT_OVER_BR_EDR__)

#include "hal_trace.h"
#include "plat_types.h"
#include "app_btgatt.h"
#include "bt_if.h"
#include "app_bt.h"

bool app_btgatt_over_br_edr_enabled(void)
{
    return btif_is_gatt_over_br_edr_enabled();
}

bool app_btgatt_is_connected(uint8_t device_id)
{
    return gatt_over_bredr_is_profile_connected(gap_conn_idx_as_hdl(device_id)) != NULL;
}

bool app_btgatt_is_connected_by_conidx(uint8_t con_idx)
{
    return gatt_over_bredr_is_profile_connected(gap_conn_idx_as_hdl(con_idx)) != NULL;
}

void app_btgatt_client_create(const bt_bdaddr_t *remote)
{
    btif_remote_device_t * remdev = NULL;
    remdev = app_bt_get_remote_dev_by_address(remote);
    if (remdev)
    {
        gatt_create_att_bearer(btif_me_get_remote_device_hci_handle(remdev));
    }
}

void app_btgatt_disconnect(uint8_t device_id)
{
    gatt_disconnect_att_bearers(gap_conn_idx_as_hdl(device_id), true, 0);
}

void app_btgatt_addsdp(uint16_t pServiceUUID, uint16_t startHandle, uint16_t endHandle)
{

}

void app_btgatt_init(void)
{

}

void app_btgatt_register_callback(btgatt_event_callback cb)
{

}

#endif
