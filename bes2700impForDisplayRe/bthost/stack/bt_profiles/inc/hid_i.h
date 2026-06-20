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
#ifndef __HID_I_H__
#define __HID_I_H__
#include "hid_service.h"
#include "btm_i.h"
#include "sdp_i.h"
#ifdef BT_HID_DEVICE
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    HID_STATE_CLOSED,
    HID_STATE_STANDBY = 1, // ready
    HID_STATE_QUERING,
    HID_STATE_CONNECTING,
    HID_STATE_WAITING_INTRUPT_CHANNEL,
    HID_STATE_CONNECTING_INTRUPT_CHANNEL,
    HID_STATE_OPEN,
    HID_STATE_DISCONNECTING,
} hid_state_t;

typedef enum {
    HID_SESSION_CLOSE,
    HID_SESSION_CONNECTING,
    HID_SESSION_OPEN,
} hid_session_state_t;

typedef enum {
    HID_PENDING_OP_NULL = 0,
    HID_PENDING_OP_GET_REPORT,
    HID_PENDING_OP_GET_PROTOCOL,
    HID_PENDING_OP_SET_REPORT,
    HID_PENDING_OP_SET_PROTOCOL,
} hid_pending_op_t;

#define HID_CTRL_OP_SUSPEND                 0x3 // enter reduced power mode
#define HID_CTRL_OP_EXIT_SUSPEND            0x4 // exit reduced power mode
#define HID_CTRL_OP_VIRTUAL_CABLE_UNPLUG    0x5

struct hid_session_t {
    uint32 l2cap_handle;
    hid_session_state_t state;
};

#define HID_BOOT_PROTOCOL_MODE      0x0
#define HID_REPORT_PROTOCOL_MODE    0x1

typedef enum {
    HID_REPORT_TYPE_OTHER = 0,
    HID_REPORT_TYPE_INPUT,
    HID_REPORT_TYPE_OUTPUT,
    HID_REPORT_TYPE_FEATURE,
} hid_report_type_enum_t;

#define HID_FRAME_DATA_MAX_LEN 64 /* DM1 is recommended packet */
extern struct hid_stack_interface_t hid_stack_if;

struct hid_report_data_t {
    uint8_t report_id;
    uint8_t data[HID_FRAME_DATA_MAX_LEN];
    uint8_t data_len;
};

struct hid_frame_t {
    uint8_t header;
    uint8_t data[HID_FRAME_DATA_MAX_LEN];
    uint8_t data_len;
    bool waiting_rsp;
    hid_pending_op_t pending_op;
};

struct hid_frame_node_t {
    struct list_node node;
    struct hid_frame_t frame;
};

struct hid_rx_msg_t {
    uint8_t msg_type;
    uint8_t param;
    uint8_t data_len;
    uint8_t *data;
};

struct hid_control_t {
    bt_bdaddr_t remote;
    bool initiator;
    bool is_hid_device_role;
    struct hid_session_t control_session;
    struct hid_session_t interrupt_session;
    hid_state_t state;
    hid_pending_op_t pending_op;
    uint8 pending_protocol_set;
    uint8 unplug_op_pending;
    uint8 local_protocol_mode;
    uint8 remote_protocol_mode;
    uint8 remote_report_descriptor_has_report_id;
    uint8 disc_reason;
    bt_hid_callback_t hid_callback;
    struct list_node ctrl_tx_list;
    struct list_node intr_tx_list;
    bool ctrl_tx_busy;
    bool intr_tx_busy;
    bool use_interrupt_channel;
    uint8_t sensor_state_feature;
};

struct hid_stack_interface_t {
    struct hid_control_t *(*hid_alloc_control_from_id)(uint8_t device_id);
    struct hid_control_t *(*hid_search_from_control_handle)(uint32_t l2cap_handle);
    struct hid_control_t *(*hid_search_from_interrupt_handle)(uint32_t l2cap_handle);
    struct hid_control_t *(*hid_search_from_remote_bdaddr)(void *remote);
    const bt_hid_custom_descriptor_t *(*hid_get_custom_descriptor)(void);
};

struct hid_ctx_input {
    struct ctx_content ctx;
    struct bdaddr_t *remote;
    struct hid_control_t *hid_ctl;
    uint32 hid_ctrl_handle;
    uint32 hid_intr_handle;
};

void hid_stack_if_init(struct hid_stack_interface_t *stack_if);

int hid_stack_init_port(void);

int hid_stack_init_device_sdp_record(void);

int hid_connect_req(uint8_t device_id, struct hid_control_t *hid_ctl);

void hid_disconnect_req(struct hid_control_t *hid_ctl);

int hid_send_get_report(struct hid_control_t *hid_ctl, hid_report_type_enum_t report_type, uint8_t report_id); // sent by HID Host;

int hid_send_set_report(struct hid_control_t *hid_ctl, hid_report_type_enum_t report_type, struct hid_report_data_t *report); // sent by HID Host

int hid_send_get_protocol(struct hid_control_t *hid_ctl); // sent by HID Host

int hid_host_send_set_protocol(struct hid_control_t *hid_ctl, bool boot_proto_mode); // sent by HID Host;

hid_state_t hid_get_state(struct hid_control_t *hid_ctl);

int hid_send_handshake(struct hid_control_t *hid_ctl, uint8_t code);

void hid_keyboard_input_report(struct hid_control_t *hid_ctl, uint8_t modifier_key, uint8_t key_code);

void hid_keyboard_send_ctrl_key(struct hid_control_t *hid_ctl, uint8_t ctrl_key);

void hid_mouse_input_report(struct hid_control_t *hid_ctl, int8_t x_pos, int8_t y_pos, uint8_t clk_buttons);

void hid_mouse_control_input_report(struct hid_control_t *hid_ctl, uint8_t ctl_buttons);

int hid_send_control(struct hid_control_t *hid_ctl, uint8_t ctrl_op);

void hid_sensor_send_input_report(struct hid_control_t *hid_ctl, const struct bt_hid_sensor_report_t *data);

void hid_consumer_send_ctrl_key(struct hid_control_t *hid_ctl, uint8_t ctrl_key);
#ifdef __cplusplus
}
#endif
#endif /* BT_HID_DEVICE */
#endif /* __HID_I_H__ */
