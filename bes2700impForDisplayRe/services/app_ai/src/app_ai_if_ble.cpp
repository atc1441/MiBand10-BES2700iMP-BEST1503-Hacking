/***************************************************************************
 *
 * Copyright 2020-2025 BES.
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
/**
 ****************************************************************************************
 * @addtogroup AI_VOICE
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#ifdef __AI_VOICE_BLE_ENABLE__
#include "bluetooth_ble_api.h"
#include "string.h"
#include "ai_transport.h"
#include "ai_thread.h"
#include "ai_control.h"
#include "app_ai_if_ble.h"
#include "app_ble.h"

bool app_ai_if_ble_check_if_notification_processing_is_busy(uint8_t conidx)
{
    return false;
}

uint16_t app_ai_if_ble_get_conhdl_from_conidx(uint8_t conidx)
{
    return bes_ble_gap_get_conhdl_from_conidx(conidx);
}

void app_ai_if_ble_disconnect_ble(uint8_t conidx)
{
    bes_ble_gap_start_disconnect(conidx);
}

void app_ai_if_ble_set_adv(uint16_t advInterval)
{
    bes_ble_gap_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
}

void app_ai_if_ble_update_conn_param(uint8_t  conidx,
                                    uint32_t min_interval_in_ms,
                                    uint32_t max_interval_in_ms,
                                    uint32_t supervision_timeout_in_ms,
                                    uint8_t  slaveLantency)
{
    bes_ble_gap_conn_update_param(conidx, min_interval_in_ms, max_interval_in_ms, supervision_timeout_in_ms, slaveLantency);
}

void app_ai_if_ble_update_conn_param_mode(bool isEnabled)
{
    bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_AI_STREAM_ON, isEnabled);
}

void app_ai_if_ble_register_data_fill_handle(void *func, bool enable)
{
    app_ble_register_advertising(BLE_AI_ADV_HANDLE, (app_ble_adv_activity_func)func);
}

void app_ai_if_ble_data_fill_enable(bool enable)
{
    bes_ble_gap_data_fill_enable(USER_AI, enable);
}

void app_ai_if_ble_disconnect_all(void)
{
    bes_ble_gap_disconnect_all();
}

void app_ai_if_ble_force_switch_adv(bool onOff)
{
    bes_ble_gap_force_switch_adv(BLE_SWITCH_USER_AI, onOff);
}

#endif
