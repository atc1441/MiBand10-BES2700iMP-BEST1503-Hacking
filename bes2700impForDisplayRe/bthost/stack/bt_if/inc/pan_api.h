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
#ifndef __BT_APP_PAN_API_H__
#define __BT_APP_PAN_API_H__

#ifdef BT_PAN_SUPPORT

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PAN_PROTOCOL_IP     = 0x0800, // Internet Protocol version 4
    PAN_PROTOCOL_ARP    = 0x0806, // Address Resolution Protocol
    PAN_PROTOCOL_IPV6   = 0x86DD, // Internet Protocol version 6
} pan_protocol_type;

typedef enum {
    PAN_PANU_NA, // connect to network access point
    PAN_PANU_GN, // connect to group ad-hoc network
    PAN_PANU_DIRECT, // direct connection to another panu
    PAN_NA_SERVICE,
    PAN_GN_SERVICE,
} pan_role_enum;

typedef enum {
    PAN_EVENT_CHANNEL_CLOSED = 1,
    PAN_EVENT_CHANNEL_OPENED,
    PAN_EVENT_ETHERNET_DATA_IND,
} pan_event_t;

typedef enum {
    PAN_STATE_CLOSED = 0,
    PAN_STATE_STANDBY,
    PAN_STATE_QUERYING,
    PAN_STATE_CONNECTING,
    PAN_STATE_BNEP_CONNECTED,
    PAN_STATE_OPENED,
    PAN_STATE_DISCONNECTING,
} pan_state_t;

typedef enum {
    PAN_SUCCESS,
    PAN_ERROR,
    PAN_SETUP_FAILED,
    PAN_BUF_FULL,
} pan_error_t;

struct pan_ethernet_data {
    void *bnep_ethernet;
};

struct pan_callback_parm_t {
    pan_event_t pan_event;
    pan_state_t pan_state;
    uint8_t error_code;
    struct pan_ethernet_data ethernet_data;
};

struct pan_ethernet_data_info;

struct btif_pan_channel_t;

typedef void (*btif_pan_callback_t)(uint8_t device_id, struct btif_pan_channel_t *pan_ctl, const struct pan_callback_parm_t *parm);

struct btif_pan_channel_t *btif_alloc_pan_channel(uint8_t device_id, pan_role_enum role);

int btif_pan_can_send_data(void);

void btif_pan_send_data_event_ind(void);

int btif_pan_send_data_sem_wait(void);

void btif_pan_init(btif_pan_callback_t pan_callback);

bt_status_t btif_pan_connect(bt_bdaddr_t *addr);

bt_status_t btif_pan_disconnect(struct btif_pan_channel_t *pan_chan);

bool btif_pan_is_connected(struct btif_pan_channel_t *pan_chan);

int btif_pan_get_state(struct btif_pan_channel_t *pan_chan);

uint16_t btif_pan_ethernet_tag_control_info(const struct pan_ethernet_data *data);

uint8_t *btif_pan_ethernet_dest_address(const struct pan_ethernet_data *data);

uint8_t *btif_pan_ethernet_source_address(const struct pan_ethernet_data *data);

uint16_t btif_pan_ethernet_protocol_type(const struct pan_ethernet_data *data);

uint8_t *btif_pan_ethernet_protocol_payload(const struct pan_ethernet_data *data);

uint16_t btif_pan_ethernet_payload_length(const struct pan_ethernet_data *data);

int btif_pan_send_ethernet_data(struct btif_pan_channel_t *pan_chan, pan_protocol_type protocol, const uint8_t *payload_data, uint16_t payload_length);

int btif_pan_send_address_specified_ethernet_data(struct btif_pan_channel_t *pan_chan, const struct pan_ethernet_data_info *data_info);

#ifdef __cplusplus
}
#endif

#endif /* BT_PAN_SUPPORT */

#endif /* __BT_APP_PAN_API_H__ */

