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
#ifndef __BNEP_I_H__
#define __BNEP_I_H__
#ifdef BT_BNEP_SUPPORT
#include "bt_common_define.h"
#include "sdp_i.h"
#include "l2cap_i.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BNEP_STATE_CLOSED,
    BNEP_STATE_STANDBY = 1, // ready
    BNEP_STATE_CHANNEL_CONNECTING,
    BNEP_STATE_CONNECTED,
    BNEP_STATE_DISCONNECTING,
} bnep_state_t;

// https://tools.ietf.org/html/rfc7042  Appendix B. Ethertypes
// https://en.wikipedia.org/wiki/EtherType

typedef enum {
    BNEP_PROTOCOL_IP     = 0x0800, // Internet Protocol version 4
    BNEP_PROTOCOL_ARP    = 0x0806, // Address Resolution Protocol
    BNEP_PROTOCOL_IPV6   = 0x86DD, // Internet Protocol version 6
    BNEP_PROTOCOL_QTAG   = 0x8100, // VLAN-tagged frame (IEEE 802.1Q Tag)
    BNEP_PROTOCOL_IPX    = 0x8137, // Internet Packet Exchange
} bnep_network_protocol_type;

typedef enum {
    BNEP_EVENT_CONN_REQ = 1,
    BNEP_EVENT_CONNECTED,
    BNEP_EVENT_DISCONNECTED,
    BNEP_EVENT_ETHERNET_DATA_IND,
    BNEP_EVENT_TX_DONE,
    BNEP_EVENT_CMD_UNKNOWN,
    BNEP_EVENT_SETUP_REQ,
    BNEP_EVENT_SETUP_RSP,
    BNEP_EVENT_PROTOCOL_FILTER_RSP,
} bnep_event_t;

#define BNEP_CTRL_CMD_NOT_UNDERSTOOD        0x00
#define BNEP_CTRL_CMD_SETUP_REQ             0x01
#define BNEP_CTRL_CMD_SETUP_RSP             0x02
#define BNEP_CTRL_CMD_FILTER_NET_TYPE_SET   0x03
#define BNEP_CTRL_CMD_FILTER_NET_TYPE_RSP   0x04
#define BNEP_CTRL_CMD_FILTER_MULTI_ADDR_SET 0x05
#define BNEP_CTRL_CMD_FILTER_MULTI_ADDR_RSP 0x06

#define BNEP_SETUP_SUCCESS 0x0000
#define BNEP_SETUP_FAILED_INVALID_DEST_SERVICE_UUID 0x0001
#define BNEP_SETUP_FAILED_INVALID_SOURCE_SERVICE_UUID 0x0002
#define BNEP_SETUP_FAILED_INVALID_SERVICE_UUID_SIZE 0x0003
#define BNEP_SETUP_FAILED_CONNECTION_NOT_ALLOWED 0x0004

#define BNEP_FILTER_NET_TYPE_SUCCESS 0x0000
#define BNEP_FILTER_NET_TYPE_UNSUPPORTED_REQUEST 0x0001
#define BNEP_FILTER_NET_TYPE_FAILED_INVALID_PROTOCOL_RANGE 0x0002
#define BNEP_FILTER_NET_TYPE_TOO_MANY_FILTERS 0x0003
#define BNEP_FILTER_NET_TYPE_UNABLE_TO_FULFILL_REQUEST_DUE_TO_SECURITY_REASONS 0x0004

struct bnep_network_protocol_type_range
{
    uint16 network_protocol_type_range_start;
    uint16 network_protocol_type_range_end;
} __attribute__ ((packed));

struct bnep_setup_req_param_t {
    uint8 size_of_one_uuid;
    uint8 *dest_service_uuid;
    uint8 *source_service_uuid;
};

#define BNEP_NO_ADDRESS_SPECIFIED               (0x00)
#define BNEP_ONLY_SOURCE_ADDRESS_SPECIFIED      (0x01)
#define BNEP_ONLY_DEST_ADDRESS_SPECIFIED        (0x02)
#define BNEP_BOTH_SOURCE_DEST_ADDRESS_SPECIFIED (BNEP_ONLY_SOURCE_ADDRESS_SPECIFIED | BNEP_ONLY_DEST_ADDRESS_SPECIFIED)

struct bnep_802_1q_tag_control_info { // virtual bridged local area networks (VLAN) tag
    uint16 vid: 12;
    uint16 canonical_format_indicator: 1;
    uint16 priority: 3;
} __attribute__ ((packed));

struct bnep_ethernet_data_ind_t {
    uint16 network_protocol_type;
    bool has_802_1q_tag_control_info;
    uint8 address_specified;
    struct bnep_802_1q_tag_control_info tci;
    struct bdaddr_t dest_address;
    struct bdaddr_t source_address;
    uint8 *bnep_payload;
    uint16 payload_length;
};

struct bnep_callback_parms_t
{
    bnep_event_t bnep_event;
    uint16 bnep_error_code;
    union {
        uint8 one_byte_data;
        const void *ptr_param;
        struct bnep_setup_req_param_t setup_req_param;
        struct bnep_ethernet_data_ind_t ethernet_data;
    } u;
};

struct bnep_ethernet_data_info {
    uint16 network_protocol_type;
    uint8 address_specified;
    struct bdaddr_t dest_address;
    struct bdaddr_t source_address;
    void *bnep_payload;
    uint16 payload_length;
};

struct bnep_conn_t;

typedef int (*bnep_callback_t)(uint8 device_id, struct bnep_conn_t *bnep_conn, const struct bnep_callback_parms_t *info);

struct bnep_conn_t {
    struct bdaddr_t remote;
    bool initiator;
    uint32 l2cap_handle;
    bnep_state_t bnep_state;
    bnep_callback_t bnep_callback;
};

int bnep_stack_init(bnep_callback_t bnep_callback);

bnep_state_t bnep_get_state(struct bnep_conn_t *bnep_conn);

bool bnep_channel_is_connected(struct bnep_conn_t *bnep_conn);

struct bnep_conn_t *bnep_create_channel(uint8 device_id, struct bdaddr_t* remote, bnep_callback_t bnep_callback);

void bnep_close_channel(uint8 device_id, struct bnep_conn_t *bnep_conn);

void bnep_send_setup_req(uint8 device_id, struct bnep_conn_t *bnep, uint16 dest_uuid, uint16 source_uuid);

void bnep_send_setup_rsp(uint8 device_id, struct bnep_conn_t *bnep, uint16 setup_rsp_msg);

void bnep_send_protocol_filter_req(uint8 device_id, struct bnep_conn_t *bnep, uint16 range_count, struct bnep_network_protocol_type_range *range);

void bnep_send_ethernet_packet(uint8 device_id, struct bnep_conn_t *bnep, const struct bnep_ethernet_data_info *data_info);

#ifdef __cplusplus
}
#endif
#endif /* BT_BNEP_SUPPORT */
#endif /* __BNEP_I_H__ */
