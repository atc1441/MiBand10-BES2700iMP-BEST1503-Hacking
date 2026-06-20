
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
#ifndef __APP_BT_HID_H__
#define __APP_BT_HID_H__
#include "hid_service.h"
#ifdef BT_HID_DEVICE
#ifdef __cplusplus
extern "C" {
#endif

typedef struct hid_control_t* hid_channel_t;

bool app_bt_hid_is_in_shutter_mode(void);
void app_bt_hid_enter_shutter_mode(void);
void app_bt_hid_exit_shutter_mode(void);
void app_bt_hid_send_capture(void);
void app_bt_hid_disconnect_all_channels(void);

bt_status_t app_bt_hid_init(void);
bt_status_t app_bt_hid_virtual_cable_unplug(const bt_bdaddr_t *remote);
bt_status_t app_bt_hid_profile_connect(const bt_bdaddr_t *remote, int capture);
bt_status_t app_bt_hid_profile_connect_v1(const bt_bdaddr_t *remote);
bt_status_t app_bt_hid_profile_disconnect(const bt_bdaddr_t *remote);
bt_status_t app_bt_hid_send_sensor_report(const bt_bdaddr_t *remote, const struct bt_hid_sensor_report_t *report);
bt_status_t app_bt_hid_process_sensor_report(const bt_bdaddr_t *remote, const struct bt_hid_sensor_report_t *report);
bt_status_t app_bt_hid_connect(const bt_bdaddr_t *remote);
bt_status_t app_bt_hid_disconnect(const bt_bdaddr_t *remote);
bt_status_t app_bt_hid_capture_process(const bt_bdaddr_t *remote);
bt_status_t app_bt_hid_send_keyboard_report(const bt_bdaddr_t *remote, uint8_t modifier_key, uint8_t key_code);
bt_status_t app_bt_hid_send_mouse_report(const bt_bdaddr_t *remote, int8_t x_pos, int8_t y_pos, uint8_t clk_buttons);
bt_status_t app_bt_hid_send_mouse_control_report(const bt_bdaddr_t *remote, uint8_t ctl_buttons);
void app_bt_hid_send_consumer_ctrl_key(uint8_t ctrl_key);
#ifdef __cplusplus
}
#endif
#endif /* BT_HID_DEVICE */
#endif /* __APP_BT_HID_H__ */
