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
#ifdef BT_PAN_SUPPORT

#include <stdio.h>
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "bluetooth.h"
#include "besbt.h"
#include "cqueue.h"
#include "me_api.h"
#include "nvrecord_bt.h"
#include "btapp.h"
#include "app_bt.h"
#include "apps.h"
#include "besbt_cfg.h"
#include "app_bt_func.h"
#include "app_bt_cmd.h"
#include "app_pan.h"
#include "pan_api.h"
#include "pan_i.h"
#include "bt_if.h"

static struct app_bt_pan_register_t *g_pan_register_callback = NULL;
static void (*g_pan_enthernet_test_handle)(uint8_t, struct btif_pan_channel_t *, const struct pan_ethernet_data *) = NULL;
static bool g_pan_web_site_access_test = false;

static void app_bt_pan_callback(uint8_t device_id, struct btif_pan_channel_t *pan_ctl, const struct pan_callback_parm_t *param)
{
    struct BT_DEVICE_T *curr_device = NULL;
    const struct pan_ethernet_data *ethernet_data = NULL;
    uint16_t protocol_type = 0;
    uint16_t payload_length = 0;
    uint8_t *payload_data = NULL;

    if (device_id == BT_DEVICE_INVALID_ID && param->pan_event == PAN_EVENT_CHANNEL_CLOSED)
    {
        // pan profile is closed due to acl created fail
        TRACE(2, "%s acl created error %x", __func__, param->error_code);
        return;
    }

    curr_device = app_bt_get_device(device_id);

    ASSERT(curr_device->pan_channel == pan_ctl, "pan device channel must match");

    TRACE(6, "(d%x) %s event %d errno %d pan %p state %d", device_id, __func__, param->pan_event, param->error_code, pan_ctl, param->pan_state);

    switch (param->pan_event)
    {
        case PAN_EVENT_CHANNEL_OPENED:
            if (param->error_code == PAN_SUCCESS)
            {
                TRACE(2, "(d%x) %s CHANNEL_OPEN!", device_id, __func__);
            }
            else
            {
                TRACE(3, "(d%x) %s CHANNEL_OPEN FAILED %d", device_id, __func__, param->error_code);
            }
            if (g_pan_register_callback && g_pan_register_callback->pan_opened)
            {
                g_pan_register_callback->pan_opened(device_id, param->error_code == PAN_SUCCESS);
            }
            g_pan_enthernet_test_handle = NULL;
            g_pan_web_site_access_test = false;
            break;
        case PAN_EVENT_CHANNEL_CLOSED:
            TRACE(3, "(d%x) %s CHANNEL_CLOSED %d", device_id, __func__, param->error_code);
            if (g_pan_register_callback && g_pan_register_callback->pan_closed)
            {
                g_pan_register_callback->pan_closed(device_id, param->error_code);
            }
            g_pan_enthernet_test_handle = NULL;
            g_pan_web_site_access_test = false;
            break;
        case PAN_EVENT_ETHERNET_DATA_IND:
            ethernet_data = &param->ethernet_data;
            protocol_type = btif_pan_ethernet_protocol_type(ethernet_data);
            payload_length = btif_pan_ethernet_payload_length(ethernet_data);
            payload_data = btif_pan_ethernet_protocol_payload(ethernet_data);
            switch (protocol_type)
            {
                case PAN_PROTOCOL_ARP:
                    if (g_pan_register_callback && g_pan_register_callback->pan_receive_ARP_protocol_data)
                    {
                        g_pan_register_callback->pan_receive_ARP_protocol_data(device_id, payload_data, payload_length);
                    }
                    break;
                case PAN_PROTOCOL_IP:
                    if (g_pan_register_callback && g_pan_register_callback->pan_receive_IPv4_protocol_data)
                    {
                        g_pan_register_callback->pan_receive_IPv4_protocol_data(device_id, payload_data, payload_length);
                    }
                    break;
                case PAN_PROTOCOL_IPV6:
                    if (g_pan_register_callback && g_pan_register_callback->pan_receive_IPv6_protocol_data)
                    {
                        g_pan_register_callback->pan_receive_IPv6_protocol_data(device_id, payload_data, payload_length);
                    }
                    break;
                default:
                    TRACE(3, "(d%x) %s protocol %04x not handled", device_id, __func__, protocol_type);
                    break;
            }
            if (g_pan_enthernet_test_handle)
            {
                g_pan_enthernet_test_handle(device_id, pan_ctl, ethernet_data);
            }
            break;
        default:
            TRACE(2, "(d%x) %s unknown event %d", device_id, __func__, param->pan_event);
            break;
    }
}

int app_bt_pan_send_ARP_protocol_data(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length)
{
    int ret;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device && curr_device->acl_is_connected)
    {
        ret = btif_pan_send_ethernet_data(curr_device->pan_channel, PAN_PROTOCOL_ARP, payload_data, payload_length);
    }
    else
    {
        TRACE(2, "(d%x) %s failed", device_id, __func__);
        ret = PAN_ERROR;
    }

    return ret;
}

int app_bt_pan_send_IPv4_protocol_data(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length)
{
    int ret;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device && curr_device->acl_is_connected)
    {
        ret = btif_pan_send_ethernet_data(curr_device->pan_channel, PAN_PROTOCOL_IP, payload_data, payload_length);
    }
    else
    {
        TRACE(2, "(d%x) %s failed", device_id, __func__);
        ret = PAN_ERROR;
    }

    return ret;
}


int app_bt_pan_send_IPv6_protocol_data(uint8_t device_id, const uint8_t *payload_data, uint16_t payload_length)
{
    int ret;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device && curr_device->acl_is_connected)
    {
        ret = btif_pan_send_ethernet_data(curr_device->pan_channel, PAN_PROTOCOL_IPV6, payload_data, payload_length);
    }
    else
    {
        TRACE(2, "(d%x) %s failed", device_id, __func__);
        ret = PAN_ERROR;
    }

    return ret;
}


int app_bt_pan_send_address_specified_protocol_data(uint8_t device_id, const struct pan_ethernet_data_info *data)
{
    int ret;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device && curr_device->acl_is_connected)
    {
        if(!app_bt_is_besbt_thread())
        {
            if(btif_pan_can_send_data())
            {
                ret = btif_pan_send_data_sem_wait();
                if(ret > 0)
                {
                    btif_pan_send_data_event_ind();
                    ret = btif_pan_send_address_specified_ethernet_data(curr_device->pan_channel, data);
                }
            }
            else
            {
                ret = btif_pan_send_data_sem_wait();
                if(ret > 0)
                {
                    btif_pan_send_data_event_ind();
                    ret = btif_pan_send_address_specified_ethernet_data(curr_device->pan_channel, data);
                }
                else
                {
                    TRACE(2, "(d%x) %s get sem failed", device_id, __func__);
                    ret = PAN_BUF_FULL;
                }
            }
        }
        else
        {
            if(btif_pan_can_send_data())
            {
                btif_pan_send_data_event_ind();
                ret = btif_pan_send_address_specified_ethernet_data(curr_device->pan_channel, data);
            }
            else
            {
                TRACE(2, "(d%x) %s tx queue full", device_id, __func__);
                ret = PAN_BUF_FULL;
            }
        }
    }
    else
    {
        TRACE(2, "(d%x) %s failed", device_id, __func__);
        ret = PAN_ERROR;
    }

    return ret;
}

void app_bt_pan_register_callbacks(struct app_bt_pan_register_t *pan_register_callbacks)
{
    g_pan_register_callback = pan_register_callbacks;
}

bool app_bt_pan_is_connected(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    bool connected = false;

    btif_osapi_lock_stack();
    if (curr_device)
    {
        connected = btif_pan_is_connected(curr_device->pan_channel);
    }
    btif_osapi_unlock_stack();

    return connected;
}

int app_bt_connect_pan_profile(bt_bdaddr_t *bdaddr)
{
    static bt_bdaddr_t remote;
    btif_device_record_t record;

    TRACE(7, "%s address %p", __func__, bdaddr);

    if(NULL != bdaddr)
    {
        remote = *bdaddr;
    }
    else
    {
        if(nv_record_enum_dev_records(0, &record))
        {
            TRACE(0, "[APP_PAN][%s]ERROR:remote BT addr is NULL", __func__);
            return -1;
        }
        remote = record.bdAddr;
    }

    TRACE(7, "%s address %02x:%02x:%02x:%02x:%02x:%02x", __func__,
        remote.address[0], remote.address[1], remote.address[2],
        remote.address[3], remote.address[4], remote.address[5]);

    app_bt_start_custom_function_in_bt_thread((uint32_t)&remote, (uint32_t)NULL, (uint32_t)btif_pan_connect);

    return 0;
}

void app_bt_disconnect_pan_profile(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    TRACE(2, "(d%x) %s", device_id, __func__);

    if (curr_device)
    {
        app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)curr_device->pan_channel, (uint32_t)NULL, (uint32_t)(uintptr_t)btif_pan_disconnect);
    }
}

static const uint8_t g_pan_local_mac_be[] = {0x31, 0x71, 0x42, 0x86, 0x53, 0xD2};
static uint8_t g_pan_requested_ip[] = {0xc0, 0xa8, 0xa7, 0x98}; // 192.168.167.152
static uint8_t g_pan_local_ip_be[] = {0xc0, 0xa8, 0xa7, 0x98}; // 192.168.167.152

static uint8_t g_pan_peer_mac_be[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t g_pan_peer_ip_be[] = {0x00, 0x00, 0x00, 0x00};

static uint8_t g_pan_submask_be[] = {0x00, 0x00, 0x00, 0x00};
static uint8_t g_pan_router_ip[] = {0x00, 0x00, 0x00, 0x00};
static uint8_t g_pan_domain_name_server[] = {0x00, 0x00, 0x00, 0x00};
static uint8_t g_pan_server_identifier[] = {0xc0, 0xa8, 0xa8, 0x64}; // 192.168.160.100

static uint8_t g_dns_resolved_ip[] = {0x00, 0x00, 0x00, 0x00};
static uint8_t g_http_dest_ip[] = {0xb6, 0xf3, 0x74, 0x74}; // 182.254.116.116

static void app_bt_pan_enthernet_test_handle(uint8_t device_id, struct btif_pan_channel_t *pan_ctl, const struct pan_ethernet_data *ethernet_data);

typedef enum {
    IP_PROTOCOL_ICMP    = 0x01, // internet control message protocol
    IP_PROTOCOL_TCP     = 0x06, // transmission control protocol
    IP_PROTOCOL_UDP     = 0x11, // user datagram protocol
    IPV6_PROTOCOL_ICMP  = 0x3a, // ICMP for IPv6
    IPV6_HOP_BY_HOP_OPT = 0x00, // IPv6 Hop-by-Hop Option
} internet_protocol_enum;

struct pan_internet_protocol_4_header {
    uint8_t header_length: 4; // real_length = header_length * 4 byte, must [5, 15], i.e. [20-byte, 60-byte]
    uint8_t protocol_version: 4;
    uint8_t type_of_service;
    uint16_t total_length; // header real length + payload length in bytes
    uint16_t identification; // all fragments have the same id
    uint16_t fragment_offset: 13;
    uint16_t more_fragment: 1;
    uint16_t dont_fragment: 1;
    uint16_t control_flag_reserved: 1;
    uint8_t time_to_live_sec;
    uint8_t payload_protocol;
    uint16_t header_checksum; // checksum entire header, initial checksum shall be set to all zeros before calculate
    uint8_t source_address[4];
    uint8_t dest_address[4];
} __attribute__ ((packed));

struct pan_ipv4_pseudo_header {
    uint8_t source_address[4];
    uint8_t dest_address[4];
    uint8_t zeroes;
    uint8_t protocol;
    uint16_t proto_length;
} __attribute__ ((packed));

struct pan_internet_protocol_6_header {
    uint8_t version;
    uint8_t traffic_class;
    uint16_t flow_label;
    uint16_t total_length;
    uint8_t payload_protocol;
    uint8_t hop_limit;
    uint8_t source_address[16];
    uint8_t dest_address[16];
} __attribute__ ((packed));

struct pan_ipv6_pseudo_header {
    uint8_t source_ipv6_address[16];
    uint8_t dest_ipv6_address[16];
    uint32_t proto_length;
    uint8_t zeroes[3];
    uint8_t protocol;
} __attribute__ ((packed));

struct pan_user_datagram_protocol_header {
    uint16_t source_port;
    uint16_t dest_port;
    uint16_t udp_total_length; // header length + payload length, min 8-byte for only header exist
    uint16_t udp_checksum; // checksumm udp header and payload together
} __attribute__ ((packed));

struct pan_transmission_control_protocol_header {
    uint16_t source_port;
    uint16_t dest_port;
    uint32_t sequence_num;
    uint32_t ack_number;
    uint16_t control_bit_final: 1;
    uint16_t control_bit_sync: 1;
    uint16_t control_bit_reset: 1;
    uint16_t control_bit_push: 1;
    uint16_t control_bit_ack: 1;
    uint16_t control_bit_urgent: 1;
    uint16_t control_bit_explicit_congestion_notification_echo: 1;
    uint16_t control_bit_congestion_window_reduce: 1;
    uint16_t control_bit_explicit_congestion_notification_nonce: 1;
    uint16_t control_bit_reserved: 3;
    uint16_t data_offset: 4; /* tcp header size in 32-bit wrods, [5, 15] -> [20, 60-byte] */
    uint16_t receive_window_size;
    uint16_t tcp_checksum;
    uint16_t urgent_pointer;
} __attribute__ ((packed));

typedef enum {
    ICMP_TYPE_ECHO_REPLY = 0,
    ICMP_TYPE_DEST_UNREACHABLE = 3,
    ICMP_TYPE_SOURCE_QUENCH = 4,
    ICMP_TYPE_REDIRECT_MESSAGE = 5,
    ICMP_TYPE_ECHO_REQUEST = 8,
    ICMP_TYPE_ROUTER_ADVERTISEMENT = 9,
    ICMP_TYPE_ROUTER_SOLICITATION = 10,
    ICMP_TYPE_TIME_EXCEEDED = 11,
    ICMP_TYPE_BAD_IP_HEADER = 12,
    ICMP_TYPE_TIMESTAMP = 13,
    ICMP_TYPE_TIMESTAMP_REPLY = 14,
    ICMP_TYPE_INFO_REQUEST = 15,
    ICMP_TYPE_INFO_REPLY = 16,
    ICMP_TYPE_ADDR_MASK_REQUEST = 17,
    ICMP_TYPE_ADDR_MASK_REPLY = 18,
    ICMP_TYPE_EXTENDED_ECHO_REQUEST = 42,
    ICMP_TYPE_EXTENDED_ECHO_REPLY = 43,
} ipv4_icmp_type_enum;

typedef enum {
    ICMP_CODE_ECHO_REPLY = 0,
    ICMP_CODE_DEST_NETWORK_UNREACHABLE = 0,
    ICMP_CODE_DEST_HOST_UNREADCHABLE = 1,
    ICMP_CODE_DEST_PROTOCOL_UNREADCHABLE = 2,
    ICMP_CODE_DEST_PORT_UNREADCHABLE = 3,
    ICMP_CODE_FRAGEMENT_REQUIRED = 4,
    ICMP_CODE_SOURCE_ROUTE_FAILED = 5,
    ICMP_CODE_DEST_NETWORK_UNKNOWN = 6,
    ICMP_CODE_DEST_HOST_UNKNOWN = 7,
    ICMP_CODE_SOURCE_HOST_ISOLATED = 8,
    ICMP_CODE_NETWORK_ADMIN_PROHIBITED = 9,
    ICMP_CODE_HOST_ADMIN_PROHHIBITED = 10,
    ICMP_CODE_NETWORK_UNREACHABLE_FOR_TOS = 11,
    ICMP_CODE_HOST_UNREACHABLE_FOR_TOS = 12,
    ICMP_CODE_COMMUNICATION_ADMIN_PROHIBITED = 13,
    ICMP_CODE_HOST_PRECEDENCE_VIOLATION = 14,
    ICMP_CODE_PRECEDENCE_CUTOFF_IN_EFFECT = 15,
    ICMP_CODE_SOURCE_QUENCH = 0,
    ICMP_CODE_REDIRECT_DATAGRAM_FOR_NETWORK = 0,
    ICMP_CODE_REDIRECT_DATAGRAM_FOR_HOST = 1,
    ICMP_CODE_REDIRECT_DATAGRAM_FOR_TOS_AND_NETWORK = 2,
    ICMP_CODE_REDIRECT_DATAGRAM_FOR_TOS_AND_HOST = 3,
    ICMP_CODE_ECHO_REQUEST = 0,
    ICMP_CODE_ROUTER_ADVERTISEMENT = 0,
    ICMP_CODE_ROUTER_SOLICITATION = 0,
    ICMP_CODE_TTL_EXPIRED_IN_TRANSIT = 0,
    ICMP_CODE_FRAGMENT_REASSEMBLY_TIME_EXCEEDED = 1,
    ICMP_CODE_POINTER_INDICATES_ERROR = 0,
    ICMP_CODE_MISSING_REQUIRED_OPTION = 1,
    ICMP_CODE_BAD_LENGTH = 2,
    ICMP_CODE_TIMESTAMP = 0,
    ICMP_CODE_TIMESTAMP_REPLY = 0,
    ICMP_CODE_INFO_REQUEST = 0,
    ICMP_CODE_INFO_REPLY = 0,
    ICMP_CODE_ADDR_MASK_REQUEST = 0,
    ICMP_CODE_ADDR_MASK_REPLY = 0,
    ICMP_CODE_EXTENDED_ECHO_REQUEST = 0,
    ICMP_CODE_EXTENDED_ECHO_REPLY_NO_ERROR = 0,
    ICMP_CODE_EXTENDED_ECHO_REPLY_MALFORMED_QUERY = 1,
    ICMP_CODE_EXTENDED_ECHO_REPLY_NO_SUCH_INTERFACE = 2,
    ICMP_CODE_EXTENDED_ECHO_REPLY_NO_SUCH_TABLE_ENTRY = 3,
    ICMP_CODE_EXTENDED_ECHO_REPLY_MULTIPLE_INTERFACES_SATISFY_QUERY = 4,
} ipv4_icmp_code_enum;

struct pan_internet_control_management_header {
    uint8_t icmp_type;
    uint8_t icmp_code;
    uint16_t icmp_checksum;
    uint8_t rest_header_data[4];
} __attribute__ ((packed));

struct pan_internet_control_management_packet {
    struct pan_internet_protocol_4_header ipv4_header;
    struct pan_internet_control_management_header icmp_header;
} __attribute__ ((packed));

typedef enum {
    IPV6_ICMP_TYPE_DEST_UNREACHABLE = 1,
    IPV6_ICMP_TYPE_PACKET_TOO_BIG = 2,
    IPV6_ICMP_TYPE_TIME_EXCEEDED = 3,
    IPV6_ICMP_TYPE_PARAMETER_PROBLEM = 4,
    IPV6_ICMP_TYPE_ECHO_REQUEST = 128,
    IPV6_ICMP_TYPE_ECHO_REPLY = 129,
    IPV6_ICMP_TYPE_MULTICAST_QUERY = 130,
    IPV6_ICMP_TYPE_MULTICAST_LISTENER_REPORT = 131,
    IPV6_ICMP_TYPE_MULTICAST_LISTENER_DONE = 132,
    IPV6_ICMP_TYPE_NDP_ROUTER_SOLICITATION = 133,
    IPV6_ICMP_TYPE_NDP_ROUTER_ADVERTISEMENT = 134,
    IPV6_ICMP_TYPE_NDP_NEIGHBOR_SOLICITATION = 135,
    IPV6_ICMP_TYPE_NDP_NEIGHBOR_ADVERTISEMENT = 136,
    IPV6_ICMP_TYPE_NDP_REDIRECT_MESSAGE = 137,
    IPV6_ICMP_TYPE_ROUTER_RENUMBERING = 138,
    IPV6_ICMP_TYPE_ICMP_NODE_INFO_QUERY = 139,
    IPV6_ICMP_TYPE_ICMP_NODE_INFO_RESPONSE = 140,
    IPV6_ICMP_TYPE_INVERSE_NEIGHBOR_DISCOVERY_SOLICITATION_MESSAGE = 141,
    IPV6_ICMP_TYPE_INVERSE_NEIGHBOR_DISCOVERY_ADVERTISEMENT_MESSAGE = 142,
    IPV6_ICMP_TYPE_MULTICAST_LISTENER_DISCOVERY_REPORTS = 143,
    IPV6_ICMP_TYPE_HOST_AGENT_ADDRESS_DISCOVERY_REQUEST = 144,
    IPV6_ICMP_TYPE_HOST_AGENT_ADDRESS_DISCOVERY_REPLY = 145,
    IPV6_ICMP_TYPE_MOBILE_PREFIX_SOLICITION = 146,
    IPV6_ICMP_TYPE_MOBILE_PREFIX_ADVERTISEMENT = 147,
    IPV6_ICMP_TYPE_CERTIFICATION_PATH_SOLICITATION = 148,
    IPV6_ICMP_TYPE_CERTIFICATION_PATH_ADVERTISEMENT = 149,
    IPV6_ICMP_TYPE_MULTICAST_ROUTER_ADVERTISEMENT = 151,
    IPV6_ICMP_TYPE_MULTICAST_ROUTER_SOLICITATION = 152,
    IPV6_ICMP_TYPE_MULTICAST_ROUTER_TERMINATION = 153,
    IPV6_ICMP_TYPE_RPL_CONTROL_MESSAGE = 155,
} ipv6_icmp_type_enum;

typedef enum {
    IPV6_ICMP_CODE_DEST_UNREACHABLE_NO_ROUTE_TO_DEST = 0,
    IPV6_ICMP_CODE_DEST_UNREACHABLE_COMMUNICATION_WITH_DEST_ADMIN_PROHIBITED = 1,
    IPV6_ICMP_CODE_DEST_UNREACHABLE_BEYOND_SCOPE_OF_SOURCE_ADDRESS = 2,
    IPV6_ICMP_CODE_DEST_UNREACHABLE_ADDRESS_UNREACHABLE = 3,
    IPV6_ICMP_CODE_DEST_UNREACHABLE_PORT_UNREACHABLE = 4,
    IPV6_ICMP_CODE_DEST_UNREACHABLE_SOURCE_ADDRESS_FAILED = 5,
    IPV6_ICMP_CODE_DEST_UNREACHABLE_REJECT_ROUTE_TO_DEST = 6,
    IPV6_ICMP_CODE_DEST_UNREACHABLE_ERROR_IN_SOURCE_ROUTING_HEADER = 7,
    IPV6_ICMP_CODE_PACKET_TOO_BIG = 0,
    IPV6_ICMP_CODE_TIME_EXCEEDED_HOP_LIMIT_EXCEEDED_IN_TRANSIT = 0,
    IPV6_ICMP_CODE_TIME_EXCEEDED_FRAGEMENT_REASSEMBLY_TIME_EXCEEDED = 1,
    IPV6_ICMP_CODE_PARAMETER_PROBLEM_ERRO_HEADER_FIELD_ENCOUNTERED = 0,
    IPV6_ICMP_CODE_PARAMETER_PROBLEM_UNRECOGNIZED_NEXT_HEADER_TYPE = 1,
    IPV6_ICMP_CODE_PARAMETER_PROBLEM_UNRECONGNIZED_IPV6_OPTION = 2,
    IPV6_ICMP_CODE_ECHO_REQUEST = 0,
    IPV6_ICMP_CODE_ECHO_REPLY = 0,
    IPV6_ICMP_CODE_MULTICAST_QUERY = 0,
    IPV6_ICMP_CODE_MULTICAST_LISTENER_REPORT = 0,
    IPV6_ICMP_CODE_MULTICAST_LISTENER_DONE = 0,
    IPV6_ICMP_CODE_NDP_ROUTER_SOLICITATION = 0,
    IPV6_ICMP_CODE_NDP_ROUTER_ADVERTISEMENT = 0,
    IPV6_ICMP_CODE_NDP_NEIGHBOR_SOLICITATION = 0,
    IPV6_ICMP_CODE_NDP_NEIGHBOR_ADVERTISEMENT = 0,
    IPV6_ICMP_CODE_NDP_REDIRECT_MESSAGE = 0,
    IPV6_ICMP_CODE_ROUTER_RENUMBERING_COMMAND = 0,
    IPV6_ICMP_CODE_ROUTER_RENUMBERING_RESULT = 1,
    IPV6_ICMP_CODE_ROUTER_RENUMBERING_SEQUENCE_RESET = 255,
    IPV6_ICMP_CODE_ICMP_NODE_INFO_QUERY_CONTAINS_IPV6_ADDRESS = 0,
    IPV6_ICMP_CODE_ICMP_NODE_INFO_QUERY_CONTAINS_A_NAME = 1,
    IPV6_ICMP_CODE_ICMP_NODE_INFO_QUERY_CONTAINS_IPV4_ADDRESS = 2,
    IPV6_ICMP_CODE_ICMP_NODE_INFO_RESPONSE_SUCCESS = 0,
    IPV6_ICMP_CODE_ICMP_NODE_INFO_RESPONSE_REFUSE = 1,
    IPV6_ICMP_CODE_ICMP_NODE_INFO_RESPONSE_UNKNOWN = 2,
    IPV6_ICMP_CODE_INVERSE_NEIGHBOR_DISCOVERY_SOLICITATION_MESSAGE = 0,
    IPV6_ICMP_CODE_INVERSE_NEIGHBOR_DISCOVERY_ADVERTISEMENT_MESSAGE = 0,
    IPV6_ICMP_CODE_MULTICAST_LISTENER_DISCOVERY_REPORTS = 0,
    IPV6_ICMP_CODE_HOST_AGENT_ADDRESS_DISCOVERY_REQUEST = 0,
    IPV6_ICMP_CODE_HOST_AGENT_ADDRESS_DISCOVERY_REPLY = 0,
    IPV6_ICMP_CODE_MOBILE_PREFIX_SOLICITION = 0,
    IPV6_ICMP_CODE_MOBILE_PREFIX_ADVERTISEMENT = 0,
    IPV6_ICMP_CODE_CERTIFICATION_PATH_SOLICITATION = 0,
    IPV6_ICMP_CODE_CERTIFICATION_PATH_ADVERTISEMENT = 0,
    IPV6_ICMP_CODE_MULTICAST_ROUTER_ADVERTISEMENT = 0,
    IPV6_ICMP_CODE_MULTICAST_ROUTER_SOLICITATION = 0,
    IPV6_ICMP_CODE_MULTICAST_ROUTER_TERMINATION = 0,
    IPV6_ICMP_CODE_RPL_CONTROL_MESSAGE = 0,
} ipv6_icmp_code_enum;

struct pan_internet_control_management_6_header {
    uint8_t icmp_type;
    uint8_t icmp_code;
    uint16_t icmp_checksum;
    uint8_t rest_header_data[4];
} __attribute__ ((packed));

struct pan_internet_control_management_6_packet {
    struct pan_internet_protocol_6_header ipv6_header;
    struct pan_internet_control_management_6_header icmp_header;
} __attribute__ ((packed));

// https://en.wikipedia.org/wiki/List_of_TCP_and_UDP_port_numbers#Well-known_ports

typedef enum {
    NS_PORT_ECHO = 7,
    NS_PORT_DAYTIME_PROTOCOL = 13,
    NS_PORT_FTP_DATA = 20,
    NS_PORT_FTP_CONTROL = 21,
    NS_PORT_SSH = 22,
    NS_PORT_TELNET = 23,
    NS_PORT_SMTP = 25, // simple mail transfer protocol
    NS_PORT_TIME_PROTOCOL = 37,
    NS_PORT_WHOIS = 43,
    NS_PORT_DNS = 53,
    NS_PORT_BOOTP_SERVER = 67,
    NS_PORT_BOOTP_CLIENT = 68,
    NS_PORT_HTTP = 80,
    NS_PORT_POP3 = 110,
    NS_PORT_SFTP = 115,
    NS_PORT_HTTPS = 443,
    NS_PORT_RPC = 530,
    NS_PORT_DHCPV6_CLIENT = 546,
    NS_PORT_DHCPV6_SERVER = 547,
    NS_PORT_FTPS_DATA = 989,
    NS_PORT_FTPS_CONTROL = 990,
} network_service_port_enum;

/**
Internet Protocol (IP) defines how devices communicate within and across
local networks on the Internet. A DHCP server can manage IP settings for
devices on its local network, e.g., by assigning IP addresses to those
devices automatically and dynamically.

DHCP operates based on the client¨Cserver model. When a computer or other
device connects to a network, the DHCP client software sends a DHCP
broadcast query requesting the necessary information. Any DHCP server on
the network may service the request. The DHCP server manages a pool of IP
addresses and information about client configuration parameters such as
default gateway, domain name, the name servers, and time servers.
*/

struct pan_dynamic_host_configuration_header {
    uint8_t op_code; // 0x01 for boot request, 0x02 for boot reply
    uint8_t hardware_type; // 0x01 for etherent
    uint8_t hardware_address_length; // 0x06
    uint8_t hops; // 0x00
    uint32_t transaction_id;
    uint16_t seconds_since_client_process_begin;
    uint16_t reserved_flags: 15;
    uint16_t broadcast_flag: 1;
    uint8_t client_ip_address[4];
    uint8_t your_ip_address[4];
    uint8_t ip_address_of_next_server[4]; // server ip address
    uint8_t relay_agent_ip_address[4]; // gateway ip address
    uint8_t client_hardware_address[16]; // only use first 6-byte
    uint8_t server_host_name[64]; // 192 octets of 0s, bootp legacy
    uint8_t boot_file_name[128]; // 192 octets of 0s, bootp legacy
    uint8_t dhcp_magic_cookie[4]; // 0x63, 0x82, 0x53, 0x63
} __attribute__ ((packed));

typedef enum {
    DHCP_DISCOBER = 0x01,
    DHCP_OFFER,
    DHCP_REQUEST,
    DHCP_DECLINE,
    DHCP_ACK,
    DHCP_NAK,
    DHCP_RELEASE,
    DHCP_INFORM,
    DHCP_FORCE_RENEW,
    DHCP_LEASE_QUERY,
    DHCP_LEASE_UNASSIGNED,
    DHCP_LEASE_UNKNOWN,
    DHCP_LEASE_ACTIVE,
    DHCP_BULK_LEASE_QUERY,
    DHCP_LEASE_QUERY_DONE,
    DHCP_ACTIVE_LEASE_QUERY,
    DHCP_LEASE_QUERY_STATUS,
    DHCP_TLS,
} dhcp_message_type_enum;

typedef enum {
    DHCP_OPTION_PAD = 0x00,
    DHCP_OPTION_SUBNET_MASK = 0x01,
    DHCP_OPTION_ROUTER_IP_ADDRESS = 0x03,
    DHCP_OPTION_DOMAIN_NAME_SERVIER = 0x06,
    DHCP_OPTION_HOST_NAME = 0x0c,
    DHCP_OPTION_DOMAIN_NAME_TO_USE_WHEN_RESOLVING_HOSTNAMES = 0x0f,
    DHCP_OPTION_INTERFACE_MTU = 0x1a,
    DHCP_OPTION_BROADCAST_ADDRESS = 0x1c,
    DHCP_OPTION_VENDOR_SPECIFIC_INFO = 0x2b,
    DHCP_OPTION_REQUESTED_IP_ADDRESS = 0x32,
    DHCP_OPTION_IP_ADDRESS_LEASE_TIME = 0x33,
    DHCP_OPTION_MESSAGE_TYPE = 0x35,
    DHCP_OPTION_SERVER_IDENTIFIER = 0x36,
    DHCP_OPTION_PARAMETER_REQUESTED_LIST = 0x37,
    DHCP_OPTION_MAX_MESSAGE_SIZE = 0x39,
    DHCP_OPTION_RENEWAL_TIME_VALUE = 0x3a,
    DHCP_OPTION_REBINDING_TIME_VALUE = 0x3b,
    DHCP_OPTION_VENDOR_CLASS_IDENTIFIER = 0x3c,
    DHCP_OPTION_CLIENT_IDENTIFIER = 0x3d,
    DHCP_OPTION_END = 0xff, // used to mark the end of the vendor option field
} dhcp_option_code_enum;

struct pan_dhcp_option_header {
    uint8_t option_code;
    uint8_t option_length;
} __attribute__ ((packed));

struct pan_dhcp_pad_option { // can be used to pad other options so that they are aligned to the word
    uint8_t option_code; // 0x00
} __attribute__ ((packed));

struct pan_dhcp_subnet_mask_option { // must be sent before the router ip address option if both are included
    uint8_t option_code; // 0x01
    uint8_t option_length; // 4-byte
    uint8_t subnet_mask[4];
} __attribute__ ((packed));

struct pan_dhcp_router_ip_address_option { // available routers, should be listed in order of preference
    uint8_t option_code; // 0x03
    uint8_t option_length; // 4-byte
    uint8_t router_ip_address[4];
} __attribute__ ((packed));

struct pan_dhcp_domain_name_server_option { // available DNS servers, should be listed in order of preference
    uint8_t option_code; // 0x06
    uint8_t option_length; // 4-byte
    uint8_t domain_name_server[4];
} __attribute__ ((packed));

struct pan_dhcp_host_name_option {
    uint8_t option_code; // 0x0c
    uint8_t option_length; // 0x0e, atleast 1-byte
    uint8_t host_name[14]; // "best-shhx-panu"
} __attribute__ ((packed));

struct pan_dhcp_requested_ip_address_option {
    uint8_t option_code; // 0x32
    uint8_t option_length; // 4-byte
    uint8_t requested_ip_address[4];
} __attribute__ ((packed));

struct pan_dhcp_ip_address_lease_time_option {
    uint8_t option_code; // 0x33
    uint8_t option_length; // 4-byte
    uint32_t ip_address_lease_time_sec;
} __attribute__ ((packed));

struct pan_dhcp_message_type_option {
    uint8_t option_code; // 0x35 for message type option
    uint8_t option_length; // 0x01
    uint8_t message_type;
} __attribute__ ((packed));

struct pan_dhcp_server_identifier_option {
    uint8_t option_code; // 0x36
    uint8_t option_length; // 4-byte
    uint8_t server_address[4];
} __attribute__ ((packed));

struct pan_dhcp_parameter_request_list_option {
    uint8_t option_code; // 0x37
    uint8_t option_length;
    uint8_t option_list[1];
} __attribute__ ((packed));

struct pan_dhcp_max_message_size_option {
    uint8_t option_code; // 0x39
    uint8_t option_length; // 0x02
    uint16_t maximum_size; // 1500 -> 0x05, 0xdc
} __attribute__ ((packed));

struct pan_dhcp_vendor_class_identifier_option {
    uint8_t option_code; // 0x3c
    uint8_t option_length; // 0x0e, at lest 1-byte
    uint8_t vendor_class_identifier[14]; // "best-shhx-dhcp"
} __attribute__ ((packed));

struct pan_dhcp_client_identifier_option {
    uint8_t option_code; // 0x3d
    uint8_t option_length; // 0x07, at least 2-byte
    uint8_t client_identifier[7];
} __attribute__ ((packed));

struct pan_dhcp_end_of_valid_option {
    uint8_t option_code; // 0xff
} __attribute__ ((packed));

struct pan_dynamic_host_configuration_protocol {
    struct pan_internet_protocol_4_header ip_header;
    struct pan_user_datagram_protocol_header udp_header;
    struct pan_dynamic_host_configuration_header dhcp_header;
} __attribute__ ((packed));

struct pan_dynamic_host_configuration_discover_option {
    struct pan_dhcp_message_type_option dhcp_message_type;
    struct pan_dhcp_max_message_size_option max_message_size;
    struct pan_dhcp_vendor_class_identifier_option class_identifier;
    struct pan_dhcp_client_identifier_option client_identifier;
    struct pan_dhcp_host_name_option host_name;
    struct pan_dhcp_parameter_request_list_option parameter_request_list;
    uint8_t request_option_list[9];
    struct pan_dhcp_end_of_valid_option end_option;
} __attribute__ ((packed));

struct pan_dynamic_host_configuration_request_option {
    struct pan_dhcp_message_type_option dhcp_message_type;
    struct pan_dhcp_max_message_size_option max_message_size;
    struct pan_dhcp_vendor_class_identifier_option class_identifier;
    struct pan_dhcp_client_identifier_option client_identifier;
    struct pan_dhcp_requested_ip_address_option requested_ip_address;
    struct pan_dhcp_server_identifier_option server_identifier;
    struct pan_dhcp_host_name_option host_name;
    struct pan_dhcp_parameter_request_list_option parameter_request_list;
    uint8_t request_option_list[9];
    struct pan_dhcp_end_of_valid_option end_option;
} __attribute__ ((packed));

#define PAN_IPV4_HEADER_MIN_LENGTH 20
#define PAN_IPV4_HEADER_MAX_LENGTH 60

struct internet_payload_info {
    uint8_t *payload_data;
    uint16_t header_length;
    uint16_t payload_length;
};

struct internet_payload_info app_bt_pan_get_ipv4_payload(struct pan_internet_protocol_4_header *header)
{
    struct internet_payload_info payload_info = {0};
    uint8_t ipv4_header_length = header->header_length * 4;

    if (ipv4_header_length < PAN_IPV4_HEADER_MIN_LENGTH || ipv4_header_length > PAN_IPV4_HEADER_MAX_LENGTH)
    {
        TRACE(2, "%s invalid ipv4 header length %d", __func__, ipv4_header_length);
        return payload_info;
    }

    payload_info.payload_data = ((uint8_t *)header) + (header->header_length * 4);
    payload_info.header_length = ipv4_header_length;
    payload_info.payload_length = PAN_BE_TO_HOST16(&header->total_length) - ipv4_header_length;

    return payload_info;
}

struct internet_payload_info app_bt_pan_get_ipv6_payload(struct pan_internet_protocol_6_header *header)
{
    struct internet_payload_info payload_info = {0};
    uint8_t ipv6_header_length = sizeof(struct pan_internet_protocol_6_header);

    payload_info.payload_data = ((uint8_t *)header) + ipv6_header_length;
    payload_info.header_length = ipv6_header_length;
    payload_info.payload_length = PAN_BE_TO_HOST16(&header->total_length) - ipv6_header_length;

    return payload_info;
}

#define PAN_TCP_HEADER_MIN_LENGTH 20
#define PAN_TCP_HEADER_MAX_LENGTH 60

struct internet_payload_info app_bt_pan_get_tcp_payload(struct pan_internet_protocol_4_header *header)
{
    struct internet_payload_info ipv4_payload_info = {0};
    struct internet_payload_info tcp_payload_info = {0};
    struct pan_transmission_control_protocol_header *tcp_header = NULL;
    uint16_t tcp_header_length = 0;

    ipv4_payload_info = app_bt_pan_get_ipv4_payload(header);

    if (!ipv4_payload_info.header_length)
    {
        return tcp_payload_info;
    }

    tcp_header = (struct pan_transmission_control_protocol_header *)(((uint8_t *)header) + ipv4_payload_info.header_length);

    tcp_header_length = tcp_header->data_offset * 4;

    if (tcp_header_length < PAN_TCP_HEADER_MIN_LENGTH || tcp_header_length > PAN_TCP_HEADER_MAX_LENGTH)
    {
        TRACE(2, "%s invalid tcp header length %d", __func__, tcp_header_length);
        return tcp_payload_info;
    }

    tcp_payload_info.header_length = tcp_header_length;
    tcp_payload_info.payload_length = ipv4_payload_info.payload_length - tcp_header_length;
    tcp_payload_info.payload_data = (((uint8_t *)tcp_header) + tcp_header_length);

    return tcp_payload_info;
}

struct internet_payload_info app_bt_pan_get_tcp_ipv6_payload(struct pan_internet_protocol_6_header *header)
{
    struct internet_payload_info ipv6_payload_info = {0};
    struct internet_payload_info tcp_payload_info = {0};
    struct pan_transmission_control_protocol_header *tcp_header = NULL;
    uint16_t tcp_header_length = 0;

    ipv6_payload_info = app_bt_pan_get_ipv6_payload(header);

    tcp_header = (struct pan_transmission_control_protocol_header *)(((uint8_t *)header) + ipv6_payload_info.header_length);

    tcp_header_length = tcp_header->data_offset * 4;

    if (tcp_header_length < PAN_TCP_HEADER_MIN_LENGTH || tcp_header_length > PAN_TCP_HEADER_MAX_LENGTH)
    {
        TRACE(2, "%s invalid tcp header length %d", __func__, tcp_header_length);
        return tcp_payload_info;
    }

    tcp_payload_info.header_length = tcp_header_length;
    tcp_payload_info.payload_length = ipv6_payload_info.payload_length - tcp_header_length;
    tcp_payload_info.payload_data = (((uint8_t *)tcp_header) + tcp_header_length);

    return tcp_payload_info;
}

uint16_t app_bt_pan_calculate_checksum(const uint8_t *pseudo_header, uint16_t pseudo_length, const uint8_t *data, uint16_t data_length)
{
    uint16_t two_byte_value = 0;
    uint32_t checksum = 0;
    uint32_t high_part_value = 0;
    const uint8_t *protocol_data = NULL;
    int i = 0;

    if (pseudo_header && pseudo_length)
    {
        protocol_data = pseudo_header;

        for (i = 0; i < pseudo_length/2; i += 1)
        {
            two_byte_value = PAN_BE_TO_HOST16(protocol_data + i * 2);
            checksum += two_byte_value;
        }
    }

    protocol_data = data;

    for (i = 0; i < data_length/2; i += 1)
    {
        two_byte_value = PAN_BE_TO_HOST16(protocol_data + i * 2);
        checksum += two_byte_value;
    }

    high_part_value = checksum >> 16;
    while (high_part_value)
    {
        checksum = (checksum & 0xffff) + high_part_value;
        high_part_value = checksum >> 16;
    }

    checksum = (~checksum);

    return checksum;
}

void app_bt_pan_write_ipv4_header_checksum(struct pan_internet_protocol_4_header *header, uint16_t ipv4_packet_total_length)
{
    uint8_t ipv4_header_length = header->header_length * 4;
    uint16_t header_checksum = 0;

    if (ipv4_header_length < PAN_IPV4_HEADER_MIN_LENGTH || ipv4_header_length > PAN_IPV4_HEADER_MAX_LENGTH)
    {
        TRACE(2, "%s invalid ipv4 header length %d", __func__, ipv4_header_length);
        return;
    }

    PAN_HOST16_TO_BE(ipv4_packet_total_length, &header->total_length);

    header->header_checksum = 0;

    header_checksum = app_bt_pan_calculate_checksum(NULL, 0, (uint8_t *)header, ipv4_header_length);

    PAN_HOST16_TO_BE(header_checksum, &header->header_checksum);
}

void app_bt_pan_write_udp_packet_checksum(struct pan_internet_protocol_4_header *header, uint16_t udp_packet_total_length)
{
    struct pan_user_datagram_protocol_header *udp_header = NULL;
    struct pan_ipv4_pseudo_header pseudo_header;
    uint16_t udp_checksum = 0;

    udp_header = (struct pan_user_datagram_protocol_header *)app_bt_pan_get_ipv4_payload(header).payload_data;

    PAN_HOST16_TO_BE(udp_packet_total_length, &udp_header->udp_total_length);

    memcpy(pseudo_header.source_address, header->source_address, sizeof(header->source_address));
    memcpy(pseudo_header.dest_address, header->dest_address, sizeof(header->dest_address));
    pseudo_header.zeroes = 0;
    pseudo_header.protocol = header->payload_protocol;
    pseudo_header.proto_length = udp_header->udp_total_length;

    udp_header->udp_checksum = 0;

    udp_checksum = app_bt_pan_calculate_checksum((uint8_t *)&pseudo_header, sizeof(pseudo_header), (uint8_t *)udp_header, udp_packet_total_length);

    PAN_HOST16_TO_BE(udp_checksum, &udp_header->udp_checksum);
}

void app_bt_pan_write_tcp_packet_checksum(struct pan_internet_protocol_4_header *header, uint16_t tcp_length)
{
    struct pan_transmission_control_protocol_header *tcp_header = NULL;
    struct pan_ipv4_pseudo_header pseudo_header;
    uint16_t tcp_checksum = 0;

    tcp_header = (struct pan_transmission_control_protocol_header *)app_bt_pan_get_ipv4_payload(header).payload_data;

    memcpy(pseudo_header.source_address, header->source_address, sizeof(header->source_address));
    memcpy(pseudo_header.dest_address, header->dest_address, sizeof(header->dest_address));
    pseudo_header.zeroes = 0;
    pseudo_header.protocol = header->payload_protocol;
    PAN_HOST16_TO_BE(tcp_length, &pseudo_header.proto_length);

    tcp_header->tcp_checksum = 0;

    tcp_checksum = app_bt_pan_calculate_checksum((uint8_t *)&pseudo_header, sizeof(pseudo_header), (uint8_t *)tcp_header, tcp_length);

    PAN_HOST16_TO_BE(tcp_checksum, &tcp_header->tcp_checksum);
}

void app_bt_pan_write_tcp_ipv6_packet_checksum(struct pan_internet_protocol_6_header *header, uint16_t tcp_length)
{
    struct pan_transmission_control_protocol_header *tcp_header = NULL;
    struct pan_ipv6_pseudo_header pseudo_header = {{0},{0},0,{0},0};
    uint16_t tcp_checksum = 0;

    tcp_header = (struct pan_transmission_control_protocol_header *)app_bt_pan_get_ipv6_payload(header).payload_data;

    memcpy(pseudo_header.source_ipv6_address, header->source_address, sizeof(header->source_address));
    memcpy(pseudo_header.dest_ipv6_address, header->dest_address, sizeof(header->dest_address));
    pseudo_header.protocol = header->payload_protocol;
    PAN_HOST16_TO_BE(tcp_length, &pseudo_header.proto_length);

    tcp_header->tcp_checksum = 0;

    tcp_checksum = app_bt_pan_calculate_checksum((uint8_t *)&pseudo_header, sizeof(pseudo_header), (uint8_t *)tcp_header, tcp_length);

    PAN_HOST16_TO_BE(tcp_checksum, &tcp_header->tcp_checksum);
}

uint32_t g_packet_identifier_seed = 0;
uint8_t g_broadcast_mac_address[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#define ICMP_ECHO_REPLY_MAX_EXTRA_SIZE (16)

void app_bt_pan_receive_icmp_packet(uint8_t device_id, struct pan_internet_control_management_packet *icmp_packet, uint16_t icmp_packet_length)
{
    pan_internet_control_management_header *icmp_header = &icmp_packet->icmp_header;

    TRACE(3, "%s type %d code %d", __func__, icmp_header->icmp_type, icmp_header->icmp_code);

    if (icmp_header->icmp_type == ICMP_TYPE_ECHO_REQUEST && icmp_header->icmp_code == ICMP_CODE_ECHO_REQUEST)
    {
        struct pan_internet_control_management_packet *icmp_reply_packet = NULL;
        uint16_t icmp_extra_data_length = icmp_packet_length - sizeof(struct pan_internet_control_management_packet);
        uint16_t icmp_checksum = 0;
        uint8_t *icmp_extra_data = NULL;

        uint8_t ipv4_icmp_echo_reply_packet[sizeof(struct pan_internet_control_management_packet) + ICMP_ECHO_REPLY_MAX_EXTRA_SIZE] = {
            0x45, // IPv4, Header Length 5 * 4 = 20-byte
            0xc0, // type of service
            0x00, 0x00, // total length
            0x00, 0x00, // identification
            0x40, 0x00, // no fragment
            0x40, // TTL
            0x01, // Protocol: Internet Control Message Protocol
            0x00, 0x00, // header checksum
            icmp_packet->ipv4_header.dest_address[0], icmp_packet->ipv4_header.dest_address[1], icmp_packet->ipv4_header.dest_address[2], icmp_packet->ipv4_header.dest_address[3], // source address
            icmp_packet->ipv4_header.source_address[0], icmp_packet->ipv4_header.source_address[1], icmp_packet->ipv4_header.source_address[2], icmp_packet->ipv4_header.source_address[3], // dest address
            ICMP_TYPE_ECHO_REPLY, // ICMP Type
            ICMP_CODE_ECHO_REPLY, // ICMP Code
            0x00, 0x00, // ICMP header and payload checksum
            icmp_header->rest_header_data[0], icmp_header->rest_header_data[1], icmp_header->rest_header_data[2], icmp_header->rest_header_data[3], // rest data
        };

        icmp_reply_packet = (struct pan_internet_control_management_packet *)ipv4_icmp_echo_reply_packet;

        icmp_extra_data = ipv4_icmp_echo_reply_packet + sizeof(struct pan_internet_control_management_packet);

        if (icmp_extra_data_length)
        {
            if (icmp_extra_data_length > ICMP_ECHO_REPLY_MAX_EXTRA_SIZE)
            {
                TRACE(3, "%s extra data too long %d %d", __func__, icmp_extra_data_length, ICMP_ECHO_REPLY_MAX_EXTRA_SIZE);
                icmp_extra_data_length = ICMP_ECHO_REPLY_MAX_EXTRA_SIZE;
            }

            memcpy(icmp_extra_data, icmp_packet + 1, icmp_extra_data_length);
        }

        app_bt_pan_write_ipv4_header_checksum(&icmp_reply_packet->ipv4_header, sizeof(struct pan_internet_control_management_packet) + icmp_extra_data_length);

        icmp_reply_packet->icmp_header.icmp_checksum = 0;

        icmp_checksum = app_bt_pan_calculate_checksum(NULL, 0, (uint8_t *)&icmp_reply_packet->icmp_header, sizeof(struct pan_internet_control_management_header) + icmp_extra_data_length);

        PAN_HOST16_TO_BE(icmp_checksum, &icmp_reply_packet->icmp_header.icmp_checksum);

        app_bt_pan_send_IPv4_protocol_data(device_id, ipv4_icmp_echo_reply_packet, sizeof(struct pan_internet_control_management_packet) + icmp_extra_data_length);
    }
}

void app_bt_pan_test_send_ICMP_echo_request(uint8_t device_id)
{
    struct pan_internet_control_management_packet *icmp_packet = NULL;
    uint16_t icmp_checksum = 0;

    uint8_t ipv4_icmp_echo_request_packet[] = {
            0x45, // IPv4, Header Length 5 * 4 = 20-byte
            0xc0, // type of service
            0x00, 0x00, // total length
            0x00, 0x00, // identification
            0x40, 0x00, // no fragment
            0x40, // TTL
            0x01, // Protocol: Internet Control Message Protocol
            0x00, 0x00, // header checksum
            g_pan_local_ip_be[0], g_pan_local_ip_be[1], g_pan_local_ip_be[2], g_pan_local_ip_be[3], // source address
            g_pan_server_identifier[0], g_pan_server_identifier[1], g_pan_server_identifier[2], g_pan_server_identifier[3], // dest address
            ICMP_TYPE_ECHO_REQUEST, // ICMP Type
            ICMP_CODE_ECHO_REQUEST, // ICMP Code
            0x00, 0x00, // ICMP header and payload checksum
            0x00, 0x00, 0x00, 0x00, // rest data
        };

    g_pan_enthernet_test_handle = app_bt_pan_enthernet_test_handle;

    icmp_packet = (struct pan_internet_control_management_packet *)ipv4_icmp_echo_request_packet;

    app_bt_pan_write_ipv4_header_checksum(&icmp_packet->ipv4_header, sizeof(ipv4_icmp_echo_request_packet));

    icmp_packet->icmp_header.icmp_checksum = 0;

    icmp_checksum = app_bt_pan_calculate_checksum(NULL, 0, (uint8_t *)&icmp_packet->icmp_header, sizeof(struct pan_internet_control_management_header));

    PAN_HOST16_TO_BE(icmp_checksum, &icmp_packet->icmp_header.icmp_checksum);

    app_bt_pan_send_IPv4_protocol_data(device_id, ipv4_icmp_echo_request_packet, sizeof(ipv4_icmp_echo_request_packet));
}

struct app_bt_pan_neighbor_solicitation_payload {
    uint8_t raw_address[16];
    uint8_t ns_type;
    uint8_t ns_length;
    uint8_t option_data[6];
} __attribute__ ((packed));

void app_bt_pan_receive_icmp_6_packet(uint8_t device_id, struct pan_internet_control_management_6_packet *icmp_packet, uint16_t icmp_packet_length)
{
    pan_internet_control_management_6_header *icmp_header = &icmp_packet->icmp_header;

    TRACE(3, "%s type %d code %d", __func__, icmp_header->icmp_type, icmp_header->icmp_code);

    if (icmp_header->icmp_type == IPV6_ICMP_TYPE_ECHO_REQUEST && icmp_header->icmp_code == IPV6_ICMP_CODE_ECHO_REQUEST)
    {
        struct pan_internet_control_management_6_packet *icmp_reply_packet = NULL;
        uint16_t icmp_extra_data_length = icmp_packet_length - sizeof(struct pan_internet_control_management_6_packet);
        uint16_t icmp_checksum = 0;
        uint16_t packet_total_length = 0;
        uint8_t *icmp_extra_data = NULL;

        uint8_t ipv6_icmp_echo_reply_packet[sizeof(struct pan_internet_control_management_6_packet) + ICMP_ECHO_REPLY_MAX_EXTRA_SIZE] = {
            0x60, // IPv6
            0x00, // traffic class
            0x00, 0x00, // flow label
            0x00, 0x00, // payload length
            0x3a, // Protocol: ICMP for IPv6
            0x01, // Hop Limit
            icmp_packet->ipv6_header.dest_address[0], icmp_packet->ipv6_header.dest_address[1], icmp_packet->ipv6_header.dest_address[2], icmp_packet->ipv6_header.dest_address[3], // source address
            icmp_packet->ipv6_header.dest_address[4], icmp_packet->ipv6_header.dest_address[5], icmp_packet->ipv6_header.dest_address[6], icmp_packet->ipv6_header.dest_address[7],
            icmp_packet->ipv6_header.dest_address[8], icmp_packet->ipv6_header.dest_address[9], icmp_packet->ipv6_header.dest_address[10], icmp_packet->ipv6_header.dest_address[11],
            icmp_packet->ipv6_header.dest_address[12], icmp_packet->ipv6_header.dest_address[13], icmp_packet->ipv6_header.dest_address[14], icmp_packet->ipv6_header.dest_address[15],
            icmp_packet->ipv6_header.source_address[0], icmp_packet->ipv6_header.source_address[1], icmp_packet->ipv6_header.source_address[2], icmp_packet->ipv6_header.source_address[3], // dest address
            icmp_packet->ipv6_header.source_address[4], icmp_packet->ipv6_header.source_address[5], icmp_packet->ipv6_header.source_address[6], icmp_packet->ipv6_header.source_address[7],
            icmp_packet->ipv6_header.source_address[8], icmp_packet->ipv6_header.source_address[9], icmp_packet->ipv6_header.source_address[10], icmp_packet->ipv6_header.source_address[11],
            icmp_packet->ipv6_header.source_address[12], icmp_packet->ipv6_header.source_address[13], icmp_packet->ipv6_header.source_address[14], icmp_packet->ipv6_header.source_address[15],
            IPV6_ICMP_TYPE_ECHO_REPLY, // ICMP Type
            IPV6_ICMP_CODE_ECHO_REPLY, // ICMP Code
            0x00, 0x00, // ICMP header and payload checksum
            icmp_header->rest_header_data[0], icmp_header->rest_header_data[1], icmp_header->rest_header_data[2], icmp_header->rest_header_data[3], // rest data
        };

        icmp_reply_packet = (struct pan_internet_control_management_6_packet *)ipv6_icmp_echo_reply_packet;

        icmp_extra_data = ipv6_icmp_echo_reply_packet + sizeof(struct pan_internet_control_management_6_packet);

        if (icmp_extra_data_length)
        {
            if (icmp_extra_data_length > ICMP_ECHO_REPLY_MAX_EXTRA_SIZE)
            {
                TRACE(3, "%s extra data too long %d %d", __func__, icmp_extra_data_length, ICMP_ECHO_REPLY_MAX_EXTRA_SIZE);
                icmp_extra_data_length = ICMP_ECHO_REPLY_MAX_EXTRA_SIZE;
            }

            memcpy(icmp_extra_data, icmp_packet + 1, icmp_extra_data_length);
        }

        packet_total_length = sizeof(struct pan_internet_control_management_6_packet) + icmp_extra_data_length;

        PAN_HOST16_TO_BE(packet_total_length, &icmp_reply_packet->ipv6_header.total_length);

        icmp_checksum = app_bt_pan_calculate_checksum(NULL, 0, (uint8_t *)&icmp_reply_packet->icmp_header, sizeof(struct pan_internet_control_management_6_header) + icmp_extra_data_length);

        PAN_HOST16_TO_BE(icmp_checksum, &icmp_reply_packet->icmp_header.icmp_checksum);

        app_bt_pan_send_IPv6_protocol_data(device_id, ipv6_icmp_echo_reply_packet, packet_total_length);
    }
    else if (icmp_header->icmp_type == IPV6_ICMP_TYPE_NDP_NEIGHBOR_SOLICITATION)
    {
        if (g_pan_web_site_access_test)
        {
            uint8_t bnep_ns_d_address[] = {0x33, 0x33, 0xff, g_pan_local_mac_be[5], g_pan_local_mac_be[4], g_pan_local_mac_be[3]};
            uint8_t zero_ipv6_address[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t ip_ff_02____01_ff[] = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, g_pan_local_mac_be[5], g_pan_local_mac_be[4], g_pan_local_mac_be[3]};
            uint8_t ns_raw_addr_fe_80[] = {0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x34, 0x56, 0xff, 0xfe, g_pan_local_mac_be[5], g_pan_local_mac_be[4], g_pan_local_mac_be[3]};

            if (memcmp(icmp_packet->ipv6_header.source_address, zero_ipv6_address, sizeof(zero_ipv6_address)) == 0 &&
                memcmp(icmp_packet->ipv6_header.dest_address, ip_ff_02____01_ff, sizeof(ip_ff_02____01_ff) - 3) == 0)
            {
                struct pan_internet_control_management_6_packet *icmp_reply_packet = NULL;
                uint16_t icmp_checksum = 0;
                uint16_t packet_total_length = 0;
                struct pan_ethernet_data_info ethernet_data;

                uint8_t ipv6_icmp_ns_reply_packet[] = {
                    0x60, // IPv6
                    0x00, // traffic class
                    0x00, 0x00, // flow label
                    0x00, 0x00, // payload length
                    0x3a, // Protocol: ICMP for IPv6
                    0xff, // Hop Limit
                    zero_ipv6_address[0], zero_ipv6_address[1], zero_ipv6_address[2], zero_ipv6_address[3], // source address
                    zero_ipv6_address[4], zero_ipv6_address[5], zero_ipv6_address[6], zero_ipv6_address[7],
                    zero_ipv6_address[8], zero_ipv6_address[9], zero_ipv6_address[10], zero_ipv6_address[11],
                    zero_ipv6_address[12], zero_ipv6_address[13], zero_ipv6_address[14], zero_ipv6_address[15],
                    ip_ff_02____01_ff[0], ip_ff_02____01_ff[1], ip_ff_02____01_ff[2], ip_ff_02____01_ff[3], // dest address
                    ip_ff_02____01_ff[4], ip_ff_02____01_ff[5], ip_ff_02____01_ff[6], ip_ff_02____01_ff[7],
                    ip_ff_02____01_ff[8], ip_ff_02____01_ff[9], ip_ff_02____01_ff[10], ip_ff_02____01_ff[11],
                    ip_ff_02____01_ff[12], ip_ff_02____01_ff[13], ip_ff_02____01_ff[14], ip_ff_02____01_ff[15],
                    IPV6_ICMP_TYPE_NDP_NEIGHBOR_SOLICITATION, // ICMP Type
                    IPV6_ICMP_CODE_NDP_NEIGHBOR_SOLICITATION, // ICMP Code
                    0x00, 0x00, // ICMP header and payload checksum
                    0x00, 0x00, 0x00, 0x00, // rest data
                    ns_raw_addr_fe_80[0], ns_raw_addr_fe_80[1], ns_raw_addr_fe_80[2], ns_raw_addr_fe_80[3], // NS raw address
                    ns_raw_addr_fe_80[4], ns_raw_addr_fe_80[5], ns_raw_addr_fe_80[6], ns_raw_addr_fe_80[7],
                    ns_raw_addr_fe_80[8], ns_raw_addr_fe_80[9], ns_raw_addr_fe_80[10], ns_raw_addr_fe_80[11],
                    ns_raw_addr_fe_80[12], ns_raw_addr_fe_80[13], ns_raw_addr_fe_80[14], ns_raw_addr_fe_80[15],
                    0x0E,
                    0x01,
                    0x65, 0x6c, 0x01, 0xc8, 0xc8, 0x18,
                };

                icmp_reply_packet = (struct pan_internet_control_management_6_packet *)ipv6_icmp_ns_reply_packet;

                packet_total_length = sizeof(ipv6_icmp_ns_reply_packet);

                PAN_HOST16_TO_BE(packet_total_length, &icmp_reply_packet->ipv6_header.total_length);

                icmp_checksum = app_bt_pan_calculate_checksum(NULL, 0, (uint8_t *)&icmp_reply_packet->icmp_header, sizeof(struct pan_internet_control_management_6_header) + sizeof(struct app_bt_pan_neighbor_solicitation_payload));

                PAN_HOST16_TO_BE(icmp_checksum, &icmp_reply_packet->icmp_header.icmp_checksum);

                ethernet_data.network_protocol_type = PAN_PROTOCOL_IPV6;
                ethernet_data.payload_length = packet_total_length;
                ethernet_data.payload_data = ipv6_icmp_ns_reply_packet;
                ethernet_data.dest_address = bnep_ns_d_address;
                ethernet_data.source_address = NULL;

                app_bt_pan_send_address_specified_protocol_data(device_id, &ethernet_data);
            }
        }
    }
}

void app_bt_pan_receive_hopopt_6_packet(uint8_t device_id, void *hopopt_packet, uint16_t packet_length)
{
    struct pan_internet_protocol_6_header *ipv6_header = (struct pan_internet_protocol_6_header *)hopopt_packet;

    if (g_pan_web_site_access_test)
    {
        uint8_t bnep_hp_d_address[] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x16};
        uint8_t zero_ipv6_address[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t ip_ff_02____00_16[] = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16};
        uint8_t ip_ff_02____01_ff[] = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, g_pan_local_mac_be[5], g_pan_local_mac_be[4], g_pan_local_mac_be[3]};
        uint8_t icmp_packet_length = 28;

        if (memcmp(ipv6_header->source_address, zero_ipv6_address, sizeof(zero_ipv6_address)) == 0 &&
            memcmp(ipv6_header->dest_address, ip_ff_02____00_16, sizeof(ip_ff_02____00_16)) == 0)
        {
            struct pan_internet_control_management_6_packet *hop_reply_packet = NULL;
            struct pan_internet_control_management_6_header *icmp_header = NULL;
            uint16_t icmp_checksum = 0;
            uint16_t packet_total_length = 0;
            struct pan_ethernet_data_info ethernet_data;

            uint8_t ipv6_hop_reply_packet[] = {
                0x60, // IPv6
                0x00, // traffic class
                0x00, 0x00, // flow label
                0x00, 0x00, // payload length
                0x00, // Protocol: IPv6 Hop-by-Hop Option
                0x01, // Hop Limit
                zero_ipv6_address[0], zero_ipv6_address[1], zero_ipv6_address[2], zero_ipv6_address[3], // source address
                zero_ipv6_address[4], zero_ipv6_address[5], zero_ipv6_address[6], zero_ipv6_address[7],
                zero_ipv6_address[8], zero_ipv6_address[9], zero_ipv6_address[10], zero_ipv6_address[11],
                zero_ipv6_address[12], zero_ipv6_address[13], zero_ipv6_address[14], zero_ipv6_address[15],
                ip_ff_02____00_16[0], ip_ff_02____00_16[1], ip_ff_02____00_16[2], ip_ff_02____00_16[3], // dest address
                ip_ff_02____00_16[4], ip_ff_02____00_16[5], ip_ff_02____00_16[6], ip_ff_02____00_16[7],
                ip_ff_02____00_16[8], ip_ff_02____00_16[9], ip_ff_02____00_16[10], ip_ff_02____00_16[11],
                ip_ff_02____00_16[12], ip_ff_02____00_16[13], ip_ff_02____00_16[14], ip_ff_02____00_16[15],
                0x3A, // Next Header: ICMP for IPv6
                0x00,
                0x05, // Router Alert
                0x02, // Length
                0x00, 0x00, // Multicast Listener Discovery
                0x01, // Pad N
                0x00, // Length
                0x8F, // Version 2 Multicast Listener Report
                0x00, // code,
                0x00, 0x00, // ICMPv6 checksum
                0x00, 0x00, 0x00, 0x01, // Number of Address Records: 1
                0x04, // Change to Exclude Mode
                0x00, // Aux Data Len: 0
                0x00, 0x00, // Number of Sources: 0
                ip_ff_02____01_ff[0], ip_ff_02____01_ff[1], ip_ff_02____01_ff[2], ip_ff_02____01_ff[3], // multicast address
                ip_ff_02____01_ff[4], ip_ff_02____01_ff[5], ip_ff_02____01_ff[6], ip_ff_02____01_ff[7],
                ip_ff_02____01_ff[8], ip_ff_02____01_ff[9], ip_ff_02____01_ff[10], ip_ff_02____01_ff[11],
                ip_ff_02____01_ff[12], ip_ff_02____01_ff[13], ip_ff_02____01_ff[14], ip_ff_02____01_ff[15],
            };

            hop_reply_packet = (struct pan_internet_control_management_6_packet *)ipv6_hop_reply_packet;

            icmp_header = (struct pan_internet_control_management_6_header *)(ipv6_hop_reply_packet + sizeof(ipv6_hop_reply_packet) - icmp_packet_length);

            packet_total_length = sizeof(ipv6_hop_reply_packet);

            PAN_HOST16_TO_BE(packet_total_length, &hop_reply_packet->ipv6_header.total_length);

            icmp_checksum = app_bt_pan_calculate_checksum(NULL, 0, (uint8_t *)icmp_header, icmp_packet_length);

            PAN_HOST16_TO_BE(icmp_checksum, &icmp_header->icmp_checksum);

            ethernet_data.network_protocol_type = PAN_PROTOCOL_IPV6;
            ethernet_data.payload_length = packet_total_length;
            ethernet_data.payload_data = ipv6_hop_reply_packet;
            ethernet_data.dest_address = bnep_hp_d_address;
            ethernet_data.source_address = NULL;

            app_bt_pan_send_address_specified_protocol_data(device_id, &ethernet_data);
        }
    }
}

// FE 80 00 00 00 00 00 00 5D 89 ED DE 11 07 EE 5D TSPX_iut_ipv6_address
static uint8_t g_pan_ndp_local_ipv6_be[] = {0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5D, 0x89, 0xED, 0xDE, 0x11, 0x07, 0xEE, 0x5D};

// FE 80 00 00 00 00 00 00 12 34 ED DE 11 07 11 5D TSPX_PTS_ipv6_address
static uint8_t g_pan_ndp_dest_ipv6_be[] = {0xFE, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34, 0xED, 0xDE, 0x11, 0x07, 0x11, 0x5D}; 

struct neighbor_solicitation_request_packet {
    struct pan_internet_control_management_6_packet icmp_packet;
    uint8_t target_address[16];
    uint8_t source_mac_address[6]; // when ipv6_header.source_address is specified
} __attribute__ ((packed));

void app_bt_pan_test_send_NDP_request(uint8_t device_id)
{
    struct neighbor_solicitation_request_packet *neighbor_solicitation_request = NULL;
    uint16_t icmp_checksum = 0;
    uint16_t packet_total_length = 0;

    uint8_t ipv6_neighbor_solicitation_packet[] = {
            0x60, // IPv6
            0x00, // traffic class
            0x00, 0x00, // flow label
            0x00, 0x00, // payload length
            0x3a, // Protocol: ICMP for IPv6
            0xff, // Hop Limit: 255
            g_pan_ndp_local_ipv6_be[0], g_pan_ndp_local_ipv6_be[1], g_pan_ndp_local_ipv6_be[2], g_pan_ndp_local_ipv6_be[3], // source ipv6 address
            g_pan_ndp_local_ipv6_be[4], g_pan_ndp_local_ipv6_be[5], g_pan_ndp_local_ipv6_be[6], g_pan_ndp_local_ipv6_be[7],
            g_pan_ndp_local_ipv6_be[8], g_pan_ndp_local_ipv6_be[9], g_pan_ndp_local_ipv6_be[10], g_pan_ndp_local_ipv6_be[11],
            g_pan_ndp_local_ipv6_be[12], g_pan_ndp_local_ipv6_be[13], g_pan_ndp_local_ipv6_be[14], g_pan_ndp_local_ipv6_be[15],
            g_pan_ndp_dest_ipv6_be[0], g_pan_ndp_dest_ipv6_be[1], g_pan_ndp_dest_ipv6_be[2], g_pan_ndp_dest_ipv6_be[3], // dest ipv6 address
            g_pan_ndp_dest_ipv6_be[4], g_pan_ndp_dest_ipv6_be[5], g_pan_ndp_dest_ipv6_be[6], g_pan_ndp_dest_ipv6_be[7],
            g_pan_ndp_dest_ipv6_be[8], g_pan_ndp_dest_ipv6_be[9], g_pan_ndp_dest_ipv6_be[10], g_pan_ndp_dest_ipv6_be[11],
            g_pan_ndp_dest_ipv6_be[12], g_pan_ndp_dest_ipv6_be[13], g_pan_ndp_dest_ipv6_be[14], g_pan_ndp_dest_ipv6_be[15],
            IPV6_ICMP_TYPE_NDP_NEIGHBOR_SOLICITATION, // ICMP Type
            IPV6_ICMP_CODE_NDP_NEIGHBOR_SOLICITATION, // ICMP Code
            0x00, 0x00, // ICMP header and payload checksum
            0x00, 0x00, 0x00, 0x00, // rest data
            g_pan_ndp_local_ipv6_be[0], g_pan_ndp_local_ipv6_be[1], g_pan_ndp_local_ipv6_be[2], g_pan_ndp_local_ipv6_be[3], // target ipv6 address
            g_pan_ndp_local_ipv6_be[4], g_pan_ndp_local_ipv6_be[5], g_pan_ndp_local_ipv6_be[6], g_pan_ndp_local_ipv6_be[7],
            g_pan_ndp_local_ipv6_be[8], g_pan_ndp_local_ipv6_be[9], g_pan_ndp_local_ipv6_be[10], g_pan_ndp_local_ipv6_be[11],
            g_pan_ndp_local_ipv6_be[12], g_pan_ndp_local_ipv6_be[13], g_pan_ndp_local_ipv6_be[14], g_pan_ndp_local_ipv6_be[15],
            g_pan_local_mac_be[0], g_pan_local_mac_be[1], g_pan_local_mac_be[2], // source mac address
            g_pan_local_mac_be[3], g_pan_local_mac_be[4], g_pan_local_mac_be[5],
        };

    neighbor_solicitation_request = (struct neighbor_solicitation_request_packet *)ipv6_neighbor_solicitation_packet;

    g_pan_enthernet_test_handle = app_bt_pan_enthernet_test_handle;

    packet_total_length = sizeof(ipv6_neighbor_solicitation_packet);

    PAN_HOST16_TO_BE(packet_total_length, &neighbor_solicitation_request->icmp_packet.ipv6_header.total_length);

    icmp_checksum = app_bt_pan_calculate_checksum(NULL, 0, (uint8_t *)&neighbor_solicitation_request->icmp_packet.icmp_header, packet_total_length - sizeof(struct pan_internet_protocol_6_header));

    PAN_HOST16_TO_BE(icmp_checksum, &neighbor_solicitation_request->icmp_packet.icmp_header.icmp_checksum);

    app_bt_pan_send_IPv6_protocol_data(device_id, ipv6_neighbor_solicitation_packet, packet_total_length);
}

void app_bt_pan_test_start_receive_enternet_data(void)
{
    g_pan_enthernet_test_handle = app_bt_pan_enthernet_test_handle;
}

static uint8_t g_app_bt_pan_test_dhcp_trans_id[] = {0xe8, 0x4e, 0xec, 0xb4};

void app_bt_pan_test_send_DHCP_protocol_discover(uint8_t device_id)
{
    struct pan_internet_protocol_4_header *ipv4_header = NULL;
    uint16_t ipv4_header_length = 0;
    uint16_t udp_length = 0;
    struct pan_ethernet_data_info ethernet_info;

    g_pan_enthernet_test_handle = app_bt_pan_enthernet_test_handle;

    uint8_t dhcp_ipv4_protocol_packet[] = {
            0x45, // IPv4, Header Length 5 * 4 = 20-byte
            0x10, // type of service
            0x00, 0x00, // total length
            0x00, 0x00, // identification
            0x40, 0x00, // no fragment
            0x40, // TTL
            0x11, // Protocol: UDP
            0x00, 0x00, // header checksum
            0x00, 0x00, 0x00, 0x00, // source address
            0xff, 0xff, 0xff, 0xff, // dest address
            0x00, 0x44, // source port: Bootstrap Protocol Client
            0x00, 0x43, // dest port: Bootstrap Protocol Server
            0x00, 0x00, // UDP total length
            0x00, 0x00, // UDP checksum
            0x01, // BOOT Request
            0x01, // Hardware Type: Ethernet
            0x06, // Hardware Address Length
            0x00, // Hops
            g_app_bt_pan_test_dhcp_trans_id[0], g_app_bt_pan_test_dhcp_trans_id[1], g_app_bt_pan_test_dhcp_trans_id[2], g_app_bt_pan_test_dhcp_trans_id[3], // transaction id
            0x00, 0x00, // Seconds Since Client Process Began
            0x00, 0x00, // Broadcast Flag
            0x00, 0x00, 0x00, 0x00, // Client IP Address
            0x00, 0x00, 0x00, 0x00, // Your IP Address
            0x00, 0x00, 0x00, 0x00, // IP Addresss of Next Server
            0x00, 0x00, 0x00, 0x00, // Relay Agent IP Address
            g_pan_local_mac_be[0], g_pan_local_mac_be[1], g_pan_local_mac_be[2],
            g_pan_local_mac_be[3], g_pan_local_mac_be[4], g_pan_local_mac_be[5],
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Client Hardware Address
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // server host name
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // boot file name
            0x63, 0x82, 0x53, 0x63, // DHCP Magic Cookie
            0x35, // >>> DHCP Message Type
            0x01, // Length
            0x01, // DHCP DISCOVER
            0x39, // >>> Max Message Size
            0x02, // Length
            0x05, 0xdc, // 1500
            0x3c, // >>> Vendor Class Identifier
            0x0e, // Length
            0x62, 0x65, 0x73, 0x74, 0x2d, 0x73, 0x68, 0x68, 0x78, 0x2d, 0x64, 0x68, 0x63, 0x70,
            0x3d, // >>> Client Identifier
            0x07, // Length
            0x01, g_pan_local_mac_be[0], g_pan_local_mac_be[1], g_pan_local_mac_be[2],
            g_pan_local_mac_be[3], g_pan_local_mac_be[4], g_pan_local_mac_be[5],
            0x0c, // >>> Host Name
            0x0e, // Length
            0x62, 0x65, 0x73, 0x74, 0x2d, 0x73, 0x68, 0x68, 0x78, 0x2d, 0x70, 0x61, 0x6e, 0x75,
            0x37, // >>> Parameter Request List
            0x09, // Length
            0x01, // Subnet Mask
            0x03, // Router IP Addresses
            0x06, // Domain Name Servers
            0x0f, // Domain Name to Use when Resolving Hostnames
            0x1a, // Interface MTU
            0x1c, // Broadcast Address
            0x33, // IP Address Lease Time
            0x3a, // Renewal Time Value
            0x3b, // Rebinding Time Value
            0xff, // End of Valid Option
        };

    ipv4_header = (struct pan_internet_protocol_4_header *)dhcp_ipv4_protocol_packet;

    ipv4_header_length = sizeof(struct pan_internet_protocol_4_header);

    udp_length = sizeof(dhcp_ipv4_protocol_packet) - ipv4_header_length;

    app_bt_pan_write_ipv4_header_checksum(ipv4_header, sizeof(dhcp_ipv4_protocol_packet));

    app_bt_pan_write_udp_packet_checksum(ipv4_header, udp_length);

    ethernet_info.network_protocol_type = PAN_PROTOCOL_IP;
    ethernet_info.payload_length = sizeof(dhcp_ipv4_protocol_packet);
    ethernet_info.payload_data = dhcp_ipv4_protocol_packet;
    ethernet_info.dest_address = g_broadcast_mac_address;
    ethernet_info.source_address = NULL;

    app_bt_pan_send_address_specified_protocol_data(device_id, &ethernet_info);
}

void app_bt_pan_test_send_DHCP_protocol_request(uint8_t device_id)
{
    struct pan_internet_protocol_4_header *ipv4_header = NULL;
    uint16_t ipv4_header_length = 0;
    uint16_t udp_length = 0;
    struct pan_ethernet_data_info ethernet_info;

    uint8_t dhcp_ipv4_protocol_packet[] = {
            0x45, // IPv4, Header Length 5 * 4 = 20-byte
            0x10, // type of service
            0x00, 0x00, // total length
            0x00, 0x00, // identification
            0x40, 0x00, // no fragment
            0x40, // TTL
            0x11, // Protocol: UDP
            0x00, 0x00, // header checksum
            0x00, 0x00, 0x00, 0x00, // source address
            0xff, 0xff, 0xff, 0xff, // dest address
            0x00, 0x44, // source port: Bootstrap Protocol Client
            0x00, 0x43, // dest port: Bootstrap Protocol Server
            0x00, 0x00, // UDP total length
            0x00, 0x00, // UDP checksum
            0x01, // BOOT Request
            0x01, // Hardware Type: Ethernet
            0x06, // Hardware Address Length
            0x00, // Hops
            g_app_bt_pan_test_dhcp_trans_id[0], g_app_bt_pan_test_dhcp_trans_id[1], g_app_bt_pan_test_dhcp_trans_id[2], g_app_bt_pan_test_dhcp_trans_id[3], // transaction id
            0x00, 0x00, // Seconds Since Client Process Began
            0x00, 0x00, // Broadcast Flag
            0x00, 0x00, 0x00, 0x00, // Client IP Address
            0x00, 0x00, 0x00, 0x00, // Your IP Address
            0x00, 0x00, 0x00, 0x00, // IP Addresss of Next Server
            0x00, 0x00, 0x00, 0x00, // Relay Agent IP Address
            g_pan_local_mac_be[0], g_pan_local_mac_be[1], g_pan_local_mac_be[2],
            g_pan_local_mac_be[3], g_pan_local_mac_be[4], g_pan_local_mac_be[5],
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Client Hardware Address
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // server host name
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // boot file name
            0x63, 0x82, 0x53, 0x63, // DHCP Magic Cookie
            0x35, // >>> DHCP Message Type
            0x01, // Length
            0x03, // DHCP REQUEST
            0x39, // >>> Max Message Size
            0x02, // Length
            0x05, 0xdc, // 1500
            0x3c, // >>> Vendor Class Identifier
            0x0e, // Length
            0x62, 0x65, 0x73, 0x74, 0x2d, 0x73, 0x68, 0x68, 0x78, 0x2d, 0x64, 0x68, 0x63, 0x70,
            0x3d, // >>> Client Identifier
            0x07, // Length
            0x01, g_pan_local_mac_be[0], g_pan_local_mac_be[1], g_pan_local_mac_be[2],
            g_pan_local_mac_be[3], g_pan_local_mac_be[4], g_pan_local_mac_be[5],
            0x32, // >>> Requested IP Address
            0x04, // Length
            g_pan_requested_ip[0], g_pan_requested_ip[1], g_pan_requested_ip[2], g_pan_requested_ip[3],
            0x36, // >>> Server Identifier
            0x04, // Length,
            g_pan_peer_ip_be[0], g_pan_peer_ip_be[1], g_pan_peer_ip_be[2], g_pan_peer_ip_be[3],
            0x0c, // >>> Host Name
            0x0e, // Length
            0x62, 0x65, 0x73, 0x74, 0x2d, 0x73, 0x68, 0x68, 0x78, 0x2d, 0x70, 0x61, 0x6e, 0x75,
            0x37, // >>> Parameter Request List
            0x09, // Length
            0x01, // Subnet Mask
            0x03, // Router IP Addresses
            0x06, // Domain Name Servers
            0x0f, // Domain Name to Use when Resolving Hostnames
            0x1a, // Interface MTU
            0x1c, // Broadcast Address
            0x33, // IP Address Lease Time
            0x3a, // Renewal Time Value
            0x3b, // Rebinding Time Value
            0xff, // End of Valid Option
        };

    ipv4_header = (struct pan_internet_protocol_4_header *)dhcp_ipv4_protocol_packet;

    ipv4_header_length = sizeof(struct pan_internet_protocol_4_header);

    udp_length = sizeof(dhcp_ipv4_protocol_packet) - ipv4_header_length;

    app_bt_pan_write_ipv4_header_checksum(ipv4_header, sizeof(dhcp_ipv4_protocol_packet));

    app_bt_pan_write_udp_packet_checksum(ipv4_header, udp_length);

    ethernet_info.network_protocol_type = PAN_PROTOCOL_IP;
    ethernet_info.payload_length = sizeof(dhcp_ipv4_protocol_packet);
    ethernet_info.payload_data = dhcp_ipv4_protocol_packet;
    ethernet_info.dest_address = g_broadcast_mac_address;
    ethernet_info.source_address = NULL;

    app_bt_pan_send_address_specified_protocol_data(device_id, &ethernet_info);
}

struct pan_address_resolution_protocol {
    uint16_t hardware_type;
    uint16_t protocol_type;
    uint8_t length_of_each_hardware_address;
    uint8_t length_of_each_protocol_address;
    uint8_t op_code[2];
    uint8_t sender_hardware_address[6];
    uint8_t sender_protocol_address[4];
    uint8_t dest_hardware_address[6];
    uint8_t dest_protocol_address[4];
} __attribute__ ((packed));

static uint8_t g_pan_arp_dest_ip_be[] = {0xc0, 0xa8, 0xa7, 0x98}; // 192.168.167.152 TSPX_iut_ip_address

void app_bt_pan_test_send_ARP_probe(uint8_t device_id)
{
    uint8_t address_resolution_protocol_payload[] = {
            0x00, 0x01, // ethernet
            0x08, 0x00, // internet protocol ver 4
            0x06,
            0x04,
            0x00, 0x01, // arp request, 0x00 0x02 for reply
            g_pan_local_mac_be[0], g_pan_local_mac_be[1], g_pan_local_mac_be[2],
            g_pan_local_mac_be[3], g_pan_local_mac_be[4], g_pan_local_mac_be[5],
            0x00, 0x00, 0x00, 0x00, // sender protocol address
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            g_pan_arp_dest_ip_be[0], g_pan_arp_dest_ip_be[1], g_pan_arp_dest_ip_be[2], g_pan_arp_dest_ip_be[3],
        };

    g_pan_enthernet_test_handle = app_bt_pan_enthernet_test_handle;

    app_bt_pan_send_ARP_protocol_data(device_id, address_resolution_protocol_payload, sizeof(address_resolution_protocol_payload));
}

void app_bt_pan_test_send_ARP_protocol_request(uint8_t device_id)
{
    uint8_t address_resolution_protocol_payload[] = {
            0x00, 0x01, // ethernet
            0x08, 0x00, // internet protocol ver 4
            0x06,
            0x04,
            0x00, 0x01, // arp request, 0x00 0x02 for reply
            g_pan_local_mac_be[0], g_pan_local_mac_be[1], g_pan_local_mac_be[2],
            g_pan_local_mac_be[3], g_pan_local_mac_be[4], g_pan_local_mac_be[5],
            g_pan_requested_ip[0], g_pan_requested_ip[1], g_pan_requested_ip[2], g_pan_requested_ip[3],
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            g_pan_peer_ip_be[0], g_pan_peer_ip_be[1], g_pan_peer_ip_be[2], g_pan_peer_ip_be[3],
        };

    g_pan_enthernet_test_handle = app_bt_pan_enthernet_test_handle;

    app_bt_pan_send_ARP_protocol_data(device_id, address_resolution_protocol_payload, sizeof(address_resolution_protocol_payload));
}

void app_bt_pan_test_send_ARP_protocol_reply(uint8_t device_id, struct pan_address_resolution_protocol *request)
{
    uint8_t zero_mac_address[] = {0x00, 0x00, 0x00, 0x00, 0x00};

    g_pan_enthernet_test_handle = app_bt_pan_enthernet_test_handle;

    if (memcmp(request->dest_protocol_address, g_pan_requested_ip, sizeof(g_pan_requested_ip)) == 0 &&
        memcmp(request->dest_hardware_address, zero_mac_address, sizeof(zero_mac_address)) == 0)
    {
        uint8_t address_resolution_protocol_payload[] = {
            0x00, 0x01, // ethernet
            0x08, 0x00, // internet protocol ver 4
            0x06,
            0x04,
            0x00, 0x02, // arp request, 0x00 0x02 for reply
            g_pan_local_mac_be[0], g_pan_local_mac_be[1], g_pan_local_mac_be[2],
            g_pan_local_mac_be[3], g_pan_local_mac_be[4], g_pan_local_mac_be[5],
            g_pan_requested_ip[0], g_pan_requested_ip[1], g_pan_requested_ip[2], g_pan_requested_ip[3],
            request->sender_hardware_address[0], request->sender_hardware_address[1], request->sender_hardware_address[2],
            request->sender_hardware_address[3], request->sender_hardware_address[4], request->sender_hardware_address[5],
            request->sender_protocol_address[0], request->sender_protocol_address[1], request->sender_protocol_address[2], request->sender_protocol_address[3], 
        };

        app_bt_pan_send_ARP_protocol_data(device_id, address_resolution_protocol_payload, sizeof(address_resolution_protocol_payload));
    }
}

struct pan_domain_name_server_header {
    uint16_t transaction_id;
    uint8_t recursion_desired: 1; // 1 Yes
    uint8_t truncation: 1; // 0 No
    uint8_t authoritative_answer: 1; // 0 No
    uint8_t dns_opcode: 4; // 0000 standard query
    uint8_t dns_indicator: 1; // 0 query, 1 response
    uint8_t response_code: 4; // 0000 no error condition
    uint8_t reserved_flag: 3; // 000
    uint8_t recursion_available: 1; // 0 No
    uint16_t question_count;
    uint16_t answer_count;
    uint16_t resource_record_count;
    uint16_t additional_record_count;
} __attribute__ ((packed));

#define DNS_MAX_QUESTION_SIZE (80)

struct pan_domain_name_server_protocol {
    struct pan_internet_protocol_4_header ip_header;
    struct pan_user_datagram_protocol_header udp_header;
    struct pan_domain_name_server_header dns_header;
    uint8_t question[DNS_MAX_QUESTION_SIZE+8];
} __attribute__ ((packed));

struct pan_domain_name_server_question_tail {
    uint16_t type_field; // 0x00 0x01 host address
    uint16_t class_field; // 0x00 0x01 internet class
} __attribute__ ((packed));

struct pan_domain_name_server_answer_tail {
    uint16_t type_field; // 0x00 0x01 host address
    uint16_t class_field; // 0x00 0x01 internet class
    uint32_t time_to_live; // TTL
    uint16_t record_length;
    uint8_t record_data[4];
} __attribute__ ((packed));

void app_bt_pan_test_send_DNS_protocol_request(uint8_t device_id, const char* domain_name, uint32_t len)
{
    //uint8_t *identifier = (uint8_t *)&g_packet_identifier_seed;
    uint8_t dns_identifier[] = {0xcd, 0x13};
    struct pan_domain_name_server_protocol *dns_packet = NULL;
    uint8_t *name_label_start = NULL;
    uint8_t *curr_name_label = NULL;
    uint8_t *type_field = NULL;
    uint8_t *class_field = NULL;
    uint16_t dns_length = 0;
    struct pan_ethernet_data_info ethernet_data;

    //g_packet_identifier_seed += 1;

    if (!domain_name || !len)
    {
        return;
    }

    if (len > DNS_MAX_QUESTION_SIZE)
    {
        TRACE(2, "%s domain name too long %d %d", __func__, len, DNS_MAX_QUESTION_SIZE);
        return;
    }

    uint8_t domain_name_server_protocol_payload[sizeof(struct pan_domain_name_server_protocol)] = {
            0x45, // IPv4, Header Length 5 * 4 = 20-byte
            0x00, // type of service
            0x00, 0x00, // total length
            0x00, 0x00, // identification
            0x40, 0x00, // no fragment
            0x40, // TTL
            0x11, // Protocol: UDP
            0x00, 0x00, // header checksum
            g_pan_local_ip_be[0], g_pan_local_ip_be[1], g_pan_local_ip_be[2], g_pan_local_ip_be[3], // source address
            g_pan_domain_name_server[0], g_pan_domain_name_server[1], g_pan_domain_name_server[2], g_pan_domain_name_server[3], // dest address
            0x50, 0x05, // source port: Random Port
            0x00, 0x35, // dest port: Domain Name Server
            0x00, 0x00, // UDP total length
            0x00, 0x00, // UDP checksum
            dns_identifier[0], dns_identifier[1], // transation id
            0x01, // standard query, recursion desired
            0x00,
            0x00, 0x01, // question count
            0x00, 0x00, // answer count
            0x00, 0x00, // resource record count
            0x00, 0x00, // additional record count
        };

    g_pan_enthernet_test_handle = app_bt_pan_enthernet_test_handle;

    dns_packet = (struct pan_domain_name_server_protocol *)domain_name_server_protocol_payload;

    name_label_start = dns_packet->question;
    curr_name_label = name_label_start + 1;

    for (uint32_t i = 0; i < len; i += 1)
    {
        if (domain_name[i] != '.')
        {
            *curr_name_label++ = domain_name[i];
        }
        else
        {
            if (curr_name_label > name_label_start + 1)
            {
                *name_label_start = curr_name_label - (name_label_start + 1);
                name_label_start = curr_name_label;
                curr_name_label = name_label_start + 1;
            }
        }
    }

    if (curr_name_label > name_label_start + 1)
    {
        *name_label_start = curr_name_label - (name_label_start + 1);
        name_label_start = curr_name_label;
    }

    *name_label_start = 0;

    type_field = name_label_start + 1;
    *type_field++ = 0x00; // Host address
    *type_field++ = 0x01;

    class_field = type_field;
    *class_field++ = 0x00; // Internet class
    *class_field++ = 0x01;

    dns_length = class_field - domain_name_server_protocol_payload;

    app_bt_pan_write_ipv4_header_checksum(&dns_packet->ip_header, dns_length);

    app_bt_pan_write_udp_packet_checksum(&dns_packet->ip_header, dns_length - sizeof(struct pan_internet_protocol_4_header));

    ethernet_data.network_protocol_type = PAN_PROTOCOL_IP;
    ethernet_data.payload_length = dns_length;
    ethernet_data.payload_data = domain_name_server_protocol_payload;
    ethernet_data.dest_address = g_pan_peer_mac_be;
    ethernet_data.source_address = NULL;

    app_bt_pan_send_address_specified_protocol_data(device_id, &ethernet_data);
}

struct pan_hypertext_transfer_protocol_header {
    struct pan_internet_protocol_4_header ip_header;
    struct pan_transmission_control_protocol_header tcp_header;
} __attribute__ ((packed));

struct pan_hypertext_transfer_protocol_ipv6_header {
    struct pan_internet_protocol_6_header ip_header;
    struct pan_transmission_control_protocol_header tcp_header;
} __attribute__ ((packed));

void app_bt_pan_test_send_HTTP_request(uint8_t device_id)
{
    struct pan_hypertext_transfer_protocol_header *http_packet = NULL;
    uint16_t packet_length = 0;

    uint8_t hypertext_transfer_protocol_request_packet[] = {
            0x45, // IPv4, Header Length 5 * 4 = 20-byte
            0x00, // type of service
            0x00, 0x00, // total length
            0x00, 0x00, // identification
            0x40, 0x00, // no fragment
            0x40, // TTL
            0x06, // Protocol: TCP
            0x00, 0x00, // header checksum
            g_pan_local_ip_be[0], g_pan_local_ip_be[1], g_pan_local_ip_be[2], g_pan_local_ip_be[3], // source address
            g_pan_web_site_access_test ? g_dns_resolved_ip[0] : g_http_dest_ip[0], // dest address
            g_pan_web_site_access_test ? g_dns_resolved_ip[1] : g_http_dest_ip[1],
            g_pan_web_site_access_test ? g_dns_resolved_ip[2] : g_http_dest_ip[2],
            g_pan_web_site_access_test ? g_dns_resolved_ip[3] : g_http_dest_ip[3], 
            0x98, 0x10, // source port: Random Port
            0x00, 0x50, // dest port: World Wide Web HTTP
            0x99, 0x85, 0x94, 0x57, // sequence number
            0x00, 0x00, 0x00, 0x01, // acknowledgment number
            0x50, // data offset 5 * 4 20-byte
            0x18, // control bits
            0xa5, 0x64, // receive window size
            0x00, 0x00, // tcp checksum
            0x00, 0x00, // urgent pointer
            0x47, 0x45, 0x54, 0x20, // "GET "
            0x2f, 0x20, // "/ "
            0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31, // "HTTP/1.1"
            0x0d, 0x0a, // "\r\n"
            0x48, 0x6f, 0x73, 0x74, 0x3a, 0x20, // "Host: "
            //0x31, 0x38, 0x32, 0x2e, 0x32, 0x35, 0x34, 0x2e, 0x31, 0x31, 0x36, 0x2e, 0x31, 0x31, 0x36, // "182.254.116.116"
            0x6d, 0x2e, 0x62, 0x61, 0x69, 0x64, 0x75, 0x2e, 0x63, 0x6f, 0x6d, // "m.baidu.com"
            0x0d, 0x0a, // "\r\n"
        };

    g_pan_enthernet_test_handle = app_bt_pan_enthernet_test_handle;

    http_packet = (struct pan_hypertext_transfer_protocol_header *)hypertext_transfer_protocol_request_packet;

    packet_length = sizeof(hypertext_transfer_protocol_request_packet);

    app_bt_pan_write_ipv4_header_checksum(&http_packet->ip_header, packet_length);

    app_bt_pan_write_tcp_packet_checksum(&http_packet->ip_header, packet_length - sizeof(struct pan_internet_protocol_4_header));

    app_bt_pan_send_IPv4_protocol_data(device_id, hypertext_transfer_protocol_request_packet, packet_length);
}

void app_bt_pan_test_send_HTTP_ipv6_request(uint8_t device_id)
{
    struct pan_hypertext_transfer_protocol_ipv6_header *http_packet = NULL;
    uint16_t packet_length = 0;

    uint8_t hypertext_transfer_protocol_ipv6_request_packet[] = {
            0x60, // IPv6
            0x00, // traffic class
            0x00, 0x00, // flow label
            0x00, 0x00, // payload length
            0x06, // Protocol: TCP
            0xff, // Hop Limit: 255
            g_pan_ndp_local_ipv6_be[0], g_pan_ndp_local_ipv6_be[1], g_pan_ndp_local_ipv6_be[2], g_pan_ndp_local_ipv6_be[3], // source ipv6 address
            g_pan_ndp_local_ipv6_be[4], g_pan_ndp_local_ipv6_be[5], g_pan_ndp_local_ipv6_be[6], g_pan_ndp_local_ipv6_be[7],
            g_pan_ndp_local_ipv6_be[8], g_pan_ndp_local_ipv6_be[9], g_pan_ndp_local_ipv6_be[10], g_pan_ndp_local_ipv6_be[11],
            g_pan_ndp_local_ipv6_be[12], g_pan_ndp_local_ipv6_be[13], g_pan_ndp_local_ipv6_be[14], g_pan_ndp_local_ipv6_be[15],
            g_pan_ndp_dest_ipv6_be[0], g_pan_ndp_dest_ipv6_be[1], g_pan_ndp_dest_ipv6_be[2], g_pan_ndp_dest_ipv6_be[3], // dest ipv6 address
            g_pan_ndp_dest_ipv6_be[4], g_pan_ndp_dest_ipv6_be[5], g_pan_ndp_dest_ipv6_be[6], g_pan_ndp_dest_ipv6_be[7],
            g_pan_ndp_dest_ipv6_be[8], g_pan_ndp_dest_ipv6_be[9], g_pan_ndp_dest_ipv6_be[10], g_pan_ndp_dest_ipv6_be[11],
            g_pan_ndp_dest_ipv6_be[12], g_pan_ndp_dest_ipv6_be[13], g_pan_ndp_dest_ipv6_be[14], g_pan_ndp_dest_ipv6_be[15],
            0x98, 0x10, // source port: Random Port
            0x00, 0x50, // dest port: World Wide Web HTTP
            0x99, 0x85, 0x94, 0x57, // sequence number
            0x00, 0x00, 0x00, 0x01, // acknowledgment number
            0x50, // data offset 5 * 4 20-byte
            0x18, // control bits
            0xa5, 0x64, // receive window size
            0x00, 0x00, // tcp checksum
            0x00, 0x00, // urgent pointer
            0x47, 0x45, 0x54, 0x20, // "GET "
            0x2f, 0x20, // "/ "
            0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31, // "HTTP/1.1"
            0x0d, 0x0a, // "\r\n"
            0x48, 0x6f, 0x73, 0x74, 0x3a, 0x20, // "Host: "
            0x31, 0x38, 0x32, 0x2e, 0x32, 0x35, 0x34, 0x2e, 0x31, 0x31, 0x36, 0x2e, 0x31, 0x31, 0x36, // "182.254.116.116"
            0x0d, 0x0a, // "\r\n"
        };

    g_pan_enthernet_test_handle = app_bt_pan_enthernet_test_handle;

    http_packet = (struct pan_hypertext_transfer_protocol_ipv6_header *)hypertext_transfer_protocol_ipv6_request_packet;

    packet_length = sizeof(hypertext_transfer_protocol_ipv6_request_packet);

    PAN_HOST16_TO_BE(packet_length, &http_packet->ip_header.total_length);

    app_bt_pan_write_tcp_ipv6_packet_checksum(&http_packet->ip_header, packet_length - sizeof(struct pan_internet_protocol_6_header));

    app_bt_pan_send_IPv6_protocol_data(device_id, hypertext_transfer_protocol_ipv6_request_packet, packet_length);
}


#define APP_BT_PAN_TEST_WAIT_TIMER_MS 1000
osTimerId g_app_bt_pan_test_wait_timer_id = 0;
static void app_bt_pan_test_wait_timer_handler(void const *n);
osTimerDef (APP_BT_PAN_TEST_WAIT_TIMER, app_bt_pan_test_wait_timer_handler);
static const char *g_app_bt_pan_test_web_site = "m.baidu.com";

typedef enum {
    APP_BT_PAN_TEST_WAIT_NULL,
    APP_BT_PAN_TEST_WAIT_DHCP_OFFER,
    APP_BT_PAN_TEST_WAIT_DHCP_ACK,
    APP_BT_PAN_TEST_WAIT_ARP_REPLY,
    APP_BT_PAN_TEST_WAIT_DNS_RESPONSE,
    APP_BT_PAN_TEST_WAIT_HTTP_RESPONSE,
} app_bt_pan_test_wait_enum;

app_bt_pan_test_wait_enum g_app_bt_pan_test_wait = APP_BT_PAN_TEST_WAIT_NULL;

static void app_bt_pan_test_wait_timer_start(void)
{
    switch (g_app_bt_pan_test_wait)
    {
        case APP_BT_PAN_TEST_WAIT_DHCP_OFFER:
            TRACE(0, "APP_BT_PAN_TEST_WAIT_DHCP_OFFER");
            break;
        case APP_BT_PAN_TEST_WAIT_DHCP_ACK:
            TRACE(0, "APP_BT_PAN_TEST_WAIT_DHCP_ACK");
            break;
        case APP_BT_PAN_TEST_WAIT_ARP_REPLY:
            TRACE(0, "APP_BT_PAN_TEST_WAIT_ARP_REPLY");
            break;
        case APP_BT_PAN_TEST_WAIT_DNS_RESPONSE:
            TRACE(0, "APP_BT_PAN_TEST_WAIT_DNS_RESPONSE");
            break;
        case APP_BT_PAN_TEST_WAIT_HTTP_RESPONSE:
            TRACE(0, "APP_BT_PAN_TEST_WAIT_HTTP_RESPONSE");
            break;
        default:
            break;
    }

    if (g_app_bt_pan_test_wait_timer_id == 0)
    {
        g_app_bt_pan_test_wait_timer_id = osTimerCreate(osTimer(APP_BT_PAN_TEST_WAIT_TIMER), osTimerOnce, NULL);
    }

    if (g_app_bt_pan_test_wait_timer_id)
    {
        osTimerStop(g_app_bt_pan_test_wait_timer_id);
        osTimerStart(g_app_bt_pan_test_wait_timer_id, APP_BT_PAN_TEST_WAIT_TIMER_MS);
    }
    else
    {
        TRACE(1, "%s invalid timer id", __func__);
    }
}

static void app_bt_pan_test_wait_timer_stop(void)
{
    if (g_app_bt_pan_test_wait_timer_id)
    {
        osTimerStop(g_app_bt_pan_test_wait_timer_id);
    }
    else
    {
        TRACE(1, "%s invalid timer id", __func__);
    }
}

static void app_bt_pan_test_wait_timer_handler(void const *n)
{
    switch (g_app_bt_pan_test_wait)
    {
        case APP_BT_PAN_TEST_WAIT_DHCP_OFFER:
            app_bt_pan_test_send_DHCP_protocol_discover(BT_DEVICE_ID_1);
            g_app_bt_pan_test_wait = APP_BT_PAN_TEST_WAIT_DHCP_OFFER;
            app_bt_pan_test_wait_timer_start();
            break;
        case APP_BT_PAN_TEST_WAIT_DHCP_ACK:
            app_bt_pan_test_send_DHCP_protocol_request(BT_DEVICE_ID_1);
            g_app_bt_pan_test_wait = APP_BT_PAN_TEST_WAIT_DHCP_ACK;
            app_bt_pan_test_wait_timer_start();
            break;
        case APP_BT_PAN_TEST_WAIT_ARP_REPLY:
            app_bt_pan_test_send_ARP_protocol_request(BT_DEVICE_ID_1);
            g_app_bt_pan_test_wait = APP_BT_PAN_TEST_WAIT_ARP_REPLY;
            app_bt_pan_test_wait_timer_start();
            break;
        case APP_BT_PAN_TEST_WAIT_DNS_RESPONSE:
            app_bt_pan_test_send_DNS_protocol_request(BT_DEVICE_ID_1, g_app_bt_pan_test_web_site, strlen(g_app_bt_pan_test_web_site));
            g_app_bt_pan_test_wait = APP_BT_PAN_TEST_WAIT_DNS_RESPONSE;
            app_bt_pan_test_wait_timer_stop();
            break;
        case APP_BT_PAN_TEST_WAIT_HTTP_RESPONSE:
            app_bt_pan_test_send_HTTP_request(BT_DEVICE_ID_1);
            g_app_bt_pan_test_wait = APP_BT_PAN_TEST_WAIT_HTTP_RESPONSE;
            app_bt_pan_test_wait_timer_start();
            break;
        default:
            break;
    }
}

void app_bt_pan_test_web_site_access(void)
{
    g_pan_web_site_access_test = true;

    app_bt_pan_test_send_DHCP_protocol_discover(BT_DEVICE_ID_1);
    g_app_bt_pan_test_wait = APP_BT_PAN_TEST_WAIT_DHCP_OFFER;
    app_bt_pan_test_wait_timer_start();
}

void app_bt_pan_handle_tcp_packet(uint8_t device_id, struct pan_transmission_control_protocol_header *tcp_header, uint16_t tcp_header_length, uint16_t tcp_payload_length)
{
    uint16_t tcp_source_port = PAN_BE_TO_HOST16(&tcp_header->source_port);
    uint16_t tcp_dest_port = PAN_BE_TO_HOST16(&tcp_header->dest_port);

    switch (tcp_source_port)
    {
        case NS_PORT_HTTP:
            {
                const char *http_protocol_data = NULL;
                http_protocol_data = ((const char *)tcp_header) + tcp_header_length;
                if (http_protocol_data)
                {
                    TRACE(3, "HTTP port %d %d method %s", tcp_source_port, tcp_dest_port, http_protocol_data);
                }
                if (g_pan_web_site_access_test)
                {
                    g_pan_web_site_access_test = false;
                    app_bt_pan_test_wait_timer_stop();
                    TRACE(1, "http response: %s", ((char *)tcp_header)+tcp_header_length);
                }
            }
            break;
        default:
            TRACE(2, "%s port %d not handled", __func__, tcp_source_port);
            break;
    }
}

void app_bt_pan_receive_tcp_packet(uint8_t device_id, struct pan_internet_protocol_4_header *packet_header, uint16_t packet_total_length)
{
    struct pan_transmission_control_protocol_header *tcp_header = (struct pan_transmission_control_protocol_header *)app_bt_pan_get_ipv4_payload(packet_header).payload_data;
    struct internet_payload_info payload_info = app_bt_pan_get_tcp_payload(packet_header);
    if (tcp_header)
    {
        app_bt_pan_handle_tcp_packet(device_id, tcp_header, payload_info.header_length, payload_info.payload_length);
    }
}

void app_bt_pan_receive_tcp_ipv6_packet(uint8_t device_id, struct pan_internet_protocol_6_header *packet_header, uint16_t packet_total_length)
{
    struct pan_transmission_control_protocol_header *tcp_header = (struct pan_transmission_control_protocol_header *)app_bt_pan_get_ipv6_payload(packet_header).payload_data;
    struct internet_payload_info payload_info = app_bt_pan_get_tcp_ipv6_payload(packet_header);
    if (tcp_header)
    {
        app_bt_pan_handle_tcp_packet(device_id, tcp_header, payload_info.header_length, payload_info.payload_length);
    }
}

void app_bt_pan_receive_udp_packet(uint8_t device_id, struct pan_user_datagram_protocol_header *udp_header, uint16_t udp_length)
{
    uint16_t udp_total_length = PAN_BE_TO_HOST16(&udp_header->udp_total_length);
    uint16_t udp_source_port = PAN_BE_TO_HOST16(&udp_header->source_port);
    uint16_t udp_dest_port = PAN_BE_TO_HOST16(&udp_header->dest_port);
    uint16_t udp_payload_length = udp_total_length - sizeof(struct pan_user_datagram_protocol_header);

    if (udp_total_length != udp_length)
    {
        TRACE(3, "%s length not match %d %d", __func__, udp_total_length, udp_length);
    }

    switch (udp_source_port)
    {
        case NS_PORT_BOOTP_SERVER:
            {
                struct pan_dynamic_host_configuration_header *dhcp_header = NULL;
                struct pan_dhcp_option_header *dhcp_option = NULL;
                uint16_t option_length = udp_payload_length - sizeof(struct pan_dynamic_host_configuration_header);
                uint16_t curr_process_len = 0;
                uint16_t option_process_len = 0;
                dhcp_header = (struct pan_dynamic_host_configuration_header *)(((uint8_t *)udp_header) + sizeof(struct pan_user_datagram_protocol_header));
                TRACE(3, "DHCP port %d %d opcode %d", udp_source_port, udp_dest_port, dhcp_header->op_code);
                memcpy(g_pan_requested_ip, dhcp_header->your_ip_address, sizeof(dhcp_header->your_ip_address));
                memcpy(g_pan_local_ip_be, dhcp_header->your_ip_address, sizeof(dhcp_header->your_ip_address));
                memcpy(g_pan_peer_ip_be, dhcp_header->ip_address_of_next_server, sizeof(dhcp_header->ip_address_of_next_server));
                dhcp_option = (struct pan_dhcp_option_header *)(((uint8_t *)(dhcp_header)) + sizeof(struct pan_dynamic_host_configuration_header));
                while (option_process_len < option_length && dhcp_option->option_code != 0x00 && dhcp_option->option_code != 0xff)
                {
                    switch (dhcp_option->option_code)
                    {
                        case DHCP_OPTION_MESSAGE_TYPE:
                            {
                                struct pan_dhcp_message_type_option *message_type_option = (struct pan_dhcp_message_type_option *)dhcp_option;
                                switch (message_type_option->message_type)
                                {
                                    case DHCP_OFFER:
                                        app_bt_pan_test_send_DHCP_protocol_request(device_id);
                                        if (g_pan_web_site_access_test)
                                        {
                                            g_app_bt_pan_test_wait = APP_BT_PAN_TEST_WAIT_DHCP_ACK;
                                            app_bt_pan_test_wait_timer_start();
                                        }
                                        break;
                                    case DHCP_ACK:
                                        TRACE(0, "DHCP ACK");
                                        if (g_pan_web_site_access_test)
                                        {
                                            
                                            TRACE(8, "DHCP ACK allocated ip %d.%d.%d.%d peer ip %d.%d.%d.%d",
                                                    g_pan_requested_ip[0], g_pan_requested_ip[1], g_pan_requested_ip[2], g_pan_requested_ip[3],
                                                    g_pan_peer_ip_be[0], g_pan_peer_ip_be[1], g_pan_peer_ip_be[2], g_pan_peer_ip_be[3]);
                                            app_bt_pan_test_send_ARP_protocol_request(device_id);
                                            g_app_bt_pan_test_wait = APP_BT_PAN_TEST_WAIT_ARP_REPLY;
                                            app_bt_pan_test_wait_timer_start();
                                        }
                                        break;
                                    case DHCP_NAK:
                                        TRACE(0, "DHCP NAK");
                                        break;
                                    default:
                                        TRACE(3, "DHCP message %d not handled", message_type_option->message_type);
                                        break;
                                }
                            }
                            break;
                        case DHCP_OPTION_SUBNET_MASK:
                            {
                                struct pan_dhcp_subnet_mask_option *subnet_mask = (struct pan_dhcp_subnet_mask_option *)dhcp_option;
                                memcpy(g_pan_submask_be, subnet_mask->subnet_mask, sizeof(subnet_mask->subnet_mask));
                            }
                            break;
                        case DHCP_OPTION_ROUTER_IP_ADDRESS:
                            {
                                struct pan_dhcp_router_ip_address_option *router_ip = (struct pan_dhcp_router_ip_address_option *)dhcp_option;
                                memcpy(g_pan_router_ip, router_ip->router_ip_address, sizeof(router_ip->router_ip_address));
                                TRACE(4, "DHCP router ip %d.%d.%d.%d", g_pan_router_ip[0], g_pan_router_ip[1], g_pan_router_ip[2], g_pan_router_ip[3]);
                            }
                            break;
                        case DHCP_OPTION_DOMAIN_NAME_SERVIER:
                            {
                                struct pan_dhcp_domain_name_server_option *domain_name_server = (struct pan_dhcp_domain_name_server_option *)dhcp_option;
                                memcpy(g_pan_domain_name_server, domain_name_server->domain_name_server, sizeof(domain_name_server->domain_name_server));
                                TRACE(4, "DHCP dns server %d.%d.%d.%d", g_pan_domain_name_server[0], g_pan_domain_name_server[1], g_pan_domain_name_server[2], g_pan_domain_name_server[3]);
                            }
                            break;
                        case DHCP_OPTION_SERVER_IDENTIFIER:
                            {
                                struct pan_dhcp_server_identifier_option *server_identifier = (struct pan_dhcp_server_identifier_option *)dhcp_option;
                                memcpy(g_pan_server_identifier, server_identifier->server_address, sizeof(server_identifier->server_address));
                                TRACE(4, "DHCP server identifier %d.%d.%d.%d", g_pan_server_identifier[0], g_pan_server_identifier[1], g_pan_server_identifier[2], g_pan_server_identifier[3]);
                            }
                            break;
                        default:
                            TRACE(3, "DHCP option %x not handled", dhcp_option->option_code);
                            break;
                    }
                    curr_process_len = sizeof(struct pan_dhcp_option_header) + dhcp_option->option_length;
                    option_process_len += curr_process_len;
                    dhcp_option = (struct pan_dhcp_option_header *)(((uint8_t *)dhcp_option) + curr_process_len);
                }
            }
            break;
        case NS_PORT_DNS:
            {
                struct pan_domain_name_server_header *dns_header = NULL;
                struct pan_domain_name_server_answer_tail *answer_tail = NULL;
                uint8_t *question_start = NULL;
                uint8_t *question_data = NULL;
                uint16_t question_length = udp_payload_length - sizeof(struct pan_domain_name_server_header);
                uint16_t processed_length = 0;
                uint16_t answer_record_len = 0;
                uint16_t question_count = 0;
                uint16_t answer_count = 0;
                dns_header = (struct pan_domain_name_server_header *)(((uint8_t *)udp_header) + sizeof(struct pan_user_datagram_protocol_header));
                question_count = PAN_BE_TO_HOST16(&dns_header->question_count);
                answer_count = PAN_BE_TO_HOST16(&dns_header->answer_count);
                TRACE(3, "DNS Response %d queston count %d answer count %d", dns_header->dns_indicator ? true : false, question_count, answer_count);
                question_start = ((uint8_t *)dns_header) + sizeof(struct pan_domain_name_server_header);
                question_data = question_start;
                while (question_count)
                {
                    while (*question_data)
                    {
                        processed_length += *question_data + 1;
                        question_data = question_start + processed_length;
                        if (processed_length >= question_length)
                        {
                            return;
                        }
                    }
                    processed_length += (1 + sizeof(struct pan_domain_name_server_question_tail));
                    question_data = question_start + processed_length;
                    question_count -= 1;
                }
                while (answer_count)
                {
                    while (*question_data)
                    {
                        processed_length += *question_data + 1;
                        TRACE(1, "DNS answer name field %s", question_data + 1);
                        question_data = question_start + processed_length;
                        if (processed_length >= question_length)
                        {
                            return;
                        }
                    }
                    answer_tail = (struct pan_domain_name_server_answer_tail *)(question_data + 1);
                    answer_record_len = PAN_BE_TO_HOST16(&answer_tail->record_length);
                    TRACE(5, "DNS answer record [%d] %d.%d.%d.%d", answer_record_len, answer_tail->record_data[0],
                        answer_tail->record_data[1], answer_tail->record_data[2], answer_tail->record_data[3]);
                    if (answer_record_len == sizeof(g_dns_resolved_ip))
                    {
                        memcpy(g_dns_resolved_ip, answer_tail->record_data, answer_record_len);
                    }
                    processed_length += (1 + sizeof(struct pan_domain_name_server_answer_tail) - sizeof(answer_tail->record_data) + answer_record_len);
                    question_data = question_start + processed_length;
                    answer_count -= 1;
                }
                if (g_pan_web_site_access_test)
                {
                    TRACE(4, "DNS resolved domain name ip address %d.%d.%d.%d",
                            g_dns_resolved_ip[0], g_dns_resolved_ip[1], g_dns_resolved_ip[2], g_dns_resolved_ip[3]);
                    app_bt_pan_test_send_HTTP_request(device_id);
                    g_app_bt_pan_test_wait = APP_BT_PAN_TEST_WAIT_HTTP_RESPONSE;
                    app_bt_pan_test_wait_timer_start();
                }
            }
            break;
        default:
            TRACE(2, "%s port %d not handled", __func__, udp_source_port);
            break;
    }
}

static void app_bt_pan_enthernet_test_handle(uint8_t device_id, struct btif_pan_channel_t *pan_ctl, const struct pan_ethernet_data *ethernet_data)
{
    uint16_t protocol_type = btif_pan_ethernet_protocol_type(ethernet_data);
    uint16_t payload_length = btif_pan_ethernet_payload_length(ethernet_data);
    uint8_t *payload_data = btif_pan_ethernet_protocol_payload(ethernet_data);

    switch (protocol_type)
    {
        case PAN_PROTOCOL_ARP:
            {
                struct pan_address_resolution_protocol *ARP = NULL;
                ARP = (struct pan_address_resolution_protocol *)payload_data;
                TRACE(5, "%s ARP %d %d op_code %02x %02x", __func__, sizeof(struct pan_address_resolution_protocol), payload_length, ARP->op_code[0], ARP->op_code[1]);
                if (ARP->op_code[0] == 0x00 && ARP->op_code[1] == 0x01) // ARP request
                {
                    TRACE(6, "ARP request: peer mac address: %02x:%02x:%02x:%02x:%02x:%02x",
                            ARP->sender_hardware_address[0], ARP->sender_hardware_address[1], ARP->sender_hardware_address[2],
                            ARP->sender_hardware_address[3], ARP->sender_hardware_address[4], ARP->sender_hardware_address[5]);
                    TRACE(4, "ARP request: peer ip address: %d.%d.%d.%d",
                            ARP->sender_protocol_address[0], ARP->sender_protocol_address[1], ARP->sender_protocol_address[2], ARP->sender_protocol_address[3]);
                    memcpy(g_pan_peer_mac_be, ARP->sender_hardware_address, sizeof(g_pan_peer_mac_be));
                    memcpy(g_pan_peer_ip_be, ARP->sender_protocol_address, sizeof(g_pan_peer_ip_be));
                    TRACE(6, "ARP request: local mac address: %02x:%02x:%02x:%02x:%02x:%02x",
                            ARP->dest_hardware_address[0], ARP->dest_hardware_address[1], ARP->dest_hardware_address[2],
                            ARP->dest_hardware_address[3], ARP->dest_hardware_address[4], ARP->dest_hardware_address[5]);
                    TRACE(4, "ARP request: local ip address: %d.%d.%d.%d",
                            ARP->dest_protocol_address[0], ARP->dest_protocol_address[1], ARP->dest_protocol_address[2], ARP->dest_protocol_address[3]);
                    if ((ARP->dest_protocol_address[0] == 0x00 && ARP->dest_protocol_address[1] == 0x00 && ARP->dest_protocol_address[2] == 0x00 && ARP->dest_protocol_address[3] == 0x00) ||
                        (ARP->dest_protocol_address[0] == 0xff && ARP->dest_protocol_address[1] == 0xff && ARP->dest_protocol_address[2] == 0xff && ARP->dest_protocol_address[3] == 0xff))
                    {
                        TRACE(0, "ARP request: local ip address is not valid");
                    }
                    else
                    {
                        memcpy(g_pan_local_ip_be, ARP->dest_protocol_address, sizeof(g_pan_local_ip_be));
                    }
                    if (g_pan_web_site_access_test && g_app_bt_pan_test_wait >= APP_BT_PAN_TEST_WAIT_DHCP_ACK)
                    {
                        app_bt_pan_test_send_ARP_protocol_reply(device_id, ARP);
                    }
                }
                else if (ARP->op_code[0] == 0x00 && ARP->op_code[1] == 0x02) // ARP reply
                {
                    TRACE(6, "ARP reply: peer mac address: %02x:%02x:%02x:%02x:%02x:%02x",
                            ARP->dest_hardware_address[0], ARP->dest_hardware_address[1], ARP->dest_hardware_address[2],
                            ARP->dest_hardware_address[3], ARP->dest_hardware_address[4], ARP->dest_hardware_address[5]);
                    memcpy(g_pan_peer_mac_be, ARP->sender_hardware_address, sizeof(g_pan_peer_mac_be));

                    if (g_pan_web_site_access_test)
                    {
                        g_app_bt_pan_test_wait = APP_BT_PAN_TEST_WAIT_DNS_RESPONSE;
                        app_bt_pan_test_wait_timer_start();
                    }
                }
            }
            break;
        case PAN_PROTOCOL_IP:
            {
                struct pan_internet_protocol_4_header *ipv4_header = (struct pan_internet_protocol_4_header *)payload_data;
                struct internet_payload_info payload = app_bt_pan_get_ipv4_payload(ipv4_header);
                if (payload.payload_length + payload.header_length != payload_length)
                {
                    TRACE(4, "%s ipv4 length not match %d %d %d", __func__, payload.header_length, payload.payload_length, payload_length);
                }
                switch (ipv4_header->payload_protocol)
                {
                    case IP_PROTOCOL_ICMP:
                        app_bt_pan_receive_icmp_packet(device_id, (struct pan_internet_control_management_packet *)payload_data, payload_length);
                        break;
                    case IP_PROTOCOL_TCP:
                        app_bt_pan_receive_tcp_packet(device_id, (struct pan_internet_protocol_4_header *)payload_data, payload_length);
                        break;
                    case IP_PROTOCOL_UDP:
                        app_bt_pan_receive_udp_packet(device_id, (struct pan_user_datagram_protocol_header *)payload.payload_data, payload.payload_length);
                        break;
                    default:
                        TRACE(2, "%s ipv4 protocol %02x not handled", __func__, ipv4_header->payload_protocol);
                        break;
                }
            }
            break;
        case PAN_PROTOCOL_IPV6:
            {
                struct pan_internet_protocol_6_header *ipv6_header = (struct pan_internet_protocol_6_header *)payload_data;
                struct internet_payload_info payload = app_bt_pan_get_ipv6_payload(ipv6_header);
                if (payload.payload_length + payload.header_length != payload_length)
                {
                    TRACE(4, "%s ipv6 length not match %d %d %d", __func__, payload.header_length, payload.payload_length, payload_length);
                }
                switch (ipv6_header->payload_protocol)
                {
                    case IPV6_HOP_BY_HOP_OPT:
                        app_bt_pan_receive_hopopt_6_packet(device_id, payload_data, payload_length);
                        break;
                    case IPV6_PROTOCOL_ICMP:
                        app_bt_pan_receive_icmp_6_packet(device_id, (struct pan_internet_control_management_6_packet *)payload_data, payload_length);
                        break;
                    case IP_PROTOCOL_TCP:
                        app_bt_pan_receive_tcp_ipv6_packet(device_id, (struct pan_internet_protocol_6_header *)payload_data, payload_length);
                        break;
                    default:
                        TRACE(2, "%s ipv6 protocol %02x not handled", __func__, ipv6_header->payload_protocol);
                        break;
                }
            }
            break;
        default:
            break;
    }
}

static void app_bt_pts_pan_connect_mobile(const char* param, uint32_t len)
{
    int bytes[sizeof(bt_bdaddr_t)] = {0};

    if (len < 17)
    {
        TRACE(0, "%s wrong len %d '%s'", __func__, len, param);
        return;
    }

    sscanf(param, "%x:%x:%x:%x:%x:%x", bytes+0, bytes+1, bytes+2, bytes+3, bytes+4, bytes+5);

    bt_bdaddr_t addr = {{
        (uint8_t)(bytes[0]&0xff),
        (uint8_t)(bytes[1]&0xff),
        (uint8_t)(bytes[2]&0xff),
        (uint8_t)(bytes[3]&0xff),
        (uint8_t)(bytes[4]&0xff),
        (uint8_t)(bytes[5]&0xff)}};

    DUMP8("%02x ", addr.address, sizeof(bt_bdaddr_t));

    app_bt_connect_pan_profile(&addr);
}

static void app_bt_pts_pan_create_channel(const char* param, uint32_t len)
{
    app_bt_connect_pan_profile(app_bt_get_pts_address());
}

static void app_bt_pts_pan_disconnect_channel(const char* param, uint32_t len)
{
    app_bt_disconnect_pan_profile(BT_DEVICE_ID_1);
}

static void app_bt_pts_pan_send_dhcp_discover(const char* param, uint32_t len)
{
    app_bt_pan_test_send_DHCP_protocol_discover(BT_DEVICE_ID_1);
}

static void app_bt_pts_pan_send_dhcp_request(const char* param, uint32_t len)
{
    app_bt_pan_test_send_DHCP_protocol_request(BT_DEVICE_ID_1);
}

static void app_bt_pts_pan_send_arp_probe(const char* param, uint32_t len)
{
    app_bt_pan_test_send_ARP_probe(BT_DEVICE_ID_1);
}

static void app_bt_pts_pan_send_arp_request(const char* param, uint32_t len)
{
    app_bt_pan_test_send_ARP_protocol_request(BT_DEVICE_ID_1);
}

static void app_bt_pts_pan_send_dns_request(const char* param, uint32_t len)
{
    const char domain_name[] = "baidu.com";
    app_bt_pan_test_send_DNS_protocol_request(BT_DEVICE_ID_1, domain_name, sizeof(domain_name)-1);
}

static void app_bt_pts_pan_send_icmp_echo_request(const char* param, uint32_t len)
{
    app_bt_pan_test_send_ICMP_echo_request(BT_DEVICE_ID_1);
}

static void app_bt_pts_pan_send_ndp_request(const char* param, uint32_t len)
{
    app_bt_pan_test_send_NDP_request(BT_DEVICE_ID_1);
}

static void app_bt_pts_pan_send_http_request(const char* param, uint32_t len)
{
    app_bt_pan_test_send_HTTP_request(BT_DEVICE_ID_1);
}

static void app_bt_pts_pan_send_http_ipv6_request(const char* param, uint32_t len)
{
    app_bt_pan_test_send_HTTP_ipv6_request(BT_DEVICE_ID_1);
}

static void app_bt_pan_test_enable_web_test(const char* param, uint32_t len)
{
    g_pan_web_site_access_test = true;
}

static void app_bt_pan_test_disable_web_test(const char* param, uint32_t len)
{
    g_pan_web_site_access_test = false;
}

static app_bt_host_cmd_table_t app_pan_test_cmd_table[] =
{
    {"pa_connect",              app_bt_pts_pan_create_channel},
    {"pa_disconnect",           app_bt_pts_pan_disconnect_channel},
    {"pa_dhcp_discover",        app_bt_pts_pan_send_dhcp_discover},
    {"pa_dhcp_request",         app_bt_pts_pan_send_dhcp_request},
    {"pa_arp_probe",            app_bt_pts_pan_send_arp_probe},
    {"pa_arp_request",          app_bt_pts_pan_send_arp_request},
    {"pa_dns_request",          app_bt_pts_pan_send_dns_request},
    {"pa_icmp_echo_req",        app_bt_pts_pan_send_icmp_echo_request},
    {"pa_ndp_request",          app_bt_pts_pan_send_ndp_request},
    {"pa_http_request",         app_bt_pts_pan_send_http_request},
    {"pa_http_ipv6_request",    app_bt_pts_pan_send_http_ipv6_request},
    {"pa_receive_data",         app_bt_pan_test_start_receive_enternet_data},
    {"pa_enable_web_test",      app_bt_pan_test_enable_web_test},
    {"pa_disable_web_test",     app_bt_pan_test_disable_web_test},
    {"pa_access_web",           app_bt_pan_test_web_site_access},
    {"pa_conn_mobile",          app_bt_pts_pan_connect_mobile},
};

void app_bt_pan_init(void)
{
    struct BT_DEVICE_T *curr_device = NULL;

    if (besbt_cfg.bt_sink_enable)
    {
        btif_pan_init(app_bt_pan_callback);

        for (int i = 0; i < BT_DEVICE_NUM; i += 1)
        {
            curr_device = app_bt_get_device(i);
            curr_device->pan_channel = btif_alloc_pan_channel(i, PAN_PANU_NA);
        }
    }

    app_bt_host_add_cmd_table(sizeof(app_pan_test_cmd_table)/sizeof(app_pan_test_cmd_table[0]),
        app_pan_test_cmd_table);
}

#endif

