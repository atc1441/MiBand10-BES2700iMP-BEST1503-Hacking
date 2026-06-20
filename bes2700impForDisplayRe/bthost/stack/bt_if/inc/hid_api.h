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
#ifndef __BTIF_HID_API_H__
#define __BTIF_HID_API_H__
#include "hid_service.h"
#ifdef BT_HID_DEVICE
#ifdef __cplusplus
extern "C" {
#endif

struct hid_control_t;

typedef enum {
    HID_DEVICE_ROLE,
    HID_HOST_ROLE,
} hid_role_enum_t;

void btif_hid_channel_init(void);

void btif_hid_init_port(void);

void btif_hid_init_sdp_record(void);

void btif_hid_intf_init(bt_hid_callback_t cb, const bt_hid_custom_descriptor_t *(*descriptor_cb)(void));

void btif_hid_init(bt_hid_callback_t cb, hid_role_enum_t role, const bt_hid_custom_descriptor_t *(*descriptor_cb)(void));

struct hid_control_t* _hid_get_control_from_id(uint8_t device_id);

struct hid_control_t *btif_hid_channel_alloc_and_init(uint8_t dev_id, bool is_device_role, bt_hid_callback_t cb);

bt_status_t btif_hid_connect(bt_bdaddr_t *addr);

void btif_hid_disconnect(struct hid_control_t *hid_ctl);

bool btif_hid_is_connected(struct hid_control_t *hid_ctl);

int btif_hid_get_state(struct hid_control_t *hid_ctl);

void btif_hid_keyboard_input_report(struct hid_control_t *hid_ctl, uint8_t modifier_key, uint8_t key_code);

void btif_hid_keyboard_send_ctrl_key(struct hid_control_t *hid_ctl, uint8_t ctrl_key);

void btif_hid_mouse_input_report(struct hid_control_t *hid_ctl, int8_t x_pos, int8_t y_pos, uint8_t clk_buttons);

void btif_hid_mouse_control_report(struct hid_control_t *hid_ctl, uint8_t ctl_buttons);

void btif_hid_send_virtual_cable_unplug(struct hid_control_t *hid_ctl);

void btif_hid_sensor_send_input_report(uint32_t a, uint32_t b, uint32_t c, uint32_t d);

void btif_hid_consumer_send_ctrl_key(struct hid_control_t *hid_ctl, uint8_t ctrl_key);

void btif_hid_host_send_set_protocl(struct hid_control_t *hid_ctl, bool boot_proto_mode);

#if defined(IBRT)
uint32_t hid_save_ctx(struct hid_control_t *hid_ctl, uint8_t *buf, uint32_t buf_len);
uint32_t hid_restore_ctx(struct hid_ctx_input *input, uint8_t hid_intr_connected);
uint32_t btif_hid_profile_save_ctx(btif_remote_device_t *rem_dev, uint8_t *buf, uint32_t buf_len);
uint32_t btif_hid_profile_restore_ctx(bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
#endif

#ifdef __cplusplus
}
#endif
#endif /* BT_HID_DEVICE */
#endif /* __BTIF_HID_API_H__ */
