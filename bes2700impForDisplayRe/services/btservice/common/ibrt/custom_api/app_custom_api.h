/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __APP_CUSTOM_API__
#define __APP_CUSTOM_API__
#include "bluetooth_bt_api.h"
#include "bt_if.h"
#include "app_ibrt_conn_evt.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef IBRT

typedef void (*codec_error_handle_t)(void);

#define WAIT_FOR_IBRT_DISCONNECT_TIMEOUT     (500)

/*
 * provide the basic interface for develop ibrt ui and add more interface in this file if which is
 * only requested by the special customer.
*/
void app_custom_ui_bt_ctx_init();

void app_custom_ui_lea_ctx_init();

void app_custom_ui_notify_bluetooth_enabled(void);

void app_custom_ui_notify_bluetooth_disabled(void);

void app_custom_ui_handler_vender_evevnt(uint8_t evt_type, uint8_t * buffer, uint32_t length);

void app_custom_ui_notify_ibrt_core_event(ibrt_conn_evt_header* evt);

void app_custom_ui_report_enhanced_rs_evt(bool allowed, ibrt_role_e role);

bool stop_ibrt_ongoing(void);

void app_custom_ui_safe_disconnect_process(uint8_t device_id);

void app_custom_ui_tws_safe_disconnect(void);

void app_custom_ui_mobiles_safe_disconnect(void);

void app_custom_ui_all_safe_disconnect(void);

void app_custom_ui_cancel_all_connection(void);

#endif

#ifdef __cplusplus
}
#endif

#endif
