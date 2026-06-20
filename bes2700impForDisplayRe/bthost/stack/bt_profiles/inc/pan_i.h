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
#ifndef __PAN_I_H__
#define __PAN_I_H__

#ifdef BT_PAN_SUPPORT

#include "bnep_i.h"
#include "pan_api.h"
#include "btm_i.h"
#include "sdp_i.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PAN_ROLE_UUID_PANU 0x1115
#define PAN_ROLE_UUID_NAP  0x1116
#define PAN_ROLE_UUID_GN   0x1117

#define PAN_ATTR_UUID_IP_SUBNET 0x0200
#define PAN_ATTR_UUID_SECURITY_DESCRIPTION 0x030A
#define PAN_ATTR_UUID_NET_ACCESS_TYPE 0x030B
#define PAN_ATTR_UUID_MAX_NET_ACCESS_RATE 0x030C
#define PAN_ATTR_UUID_IPV4_SUBNET 0x030D
#define PAN_ATTR_UUID_IPV6_SUBNET 0x030E

#define PAN_SECURITY_DESCRIPTION_NONE 0x0000
#define PAN_SECURITY_SERVICE_LEVEL_ENFORCE 0x0001
#define PAN_SECURITY_802_1X_SECURITY 0x0002

#define PAN_NAP_SUPPORT_PSTN 0x0000
#define PAN_NAP_SUPPORT_ISDN 0x0001
#define PAN_NAP_SUPPORT_DSL  0x0002
#define PAN_NAP_SUPPORT_CABLE_MODEM 0x0003
#define PAN_NAP_SUPPORT_10MB_ETHERNET 0x0004
#define PAN_NAP_SUPPORT_100MB_ETHERNET 0x0005
#define PAN_NAP_SUPPORT_4MB_TOKEN_RING 0x0006
#define PAN_NAP_SUPPORT_16MB_TOKEN_RING 0x0007
#define PAN_NAP_SUPPORT_100MB_TOKEN_RING 0x0008
#define PAN_NAP_SUPPORT_FDDI 0x0009
#define PAN_NAP_SUPPORT_GSM 0x000a
#define PAN_NAP_SUPPORT_CDMA 0x000b
#define PAN_NAP_SUPPORT_GPRS 0x000c
#define PAN_NAP_SUPPORT_3G_CELLULAR 0x000d
#define PAN_NAP_SUPPORT_OTHER 0xfffe

struct pan_control_t;

typedef void (*pan_callback_t)(uint8 device_id, struct pan_control_t *pan_ctl, const struct pan_callback_parm_t *parm);

struct pan_control_t {
    struct bnep_conn_t *bnep_conn;
    struct bdaddr_t remote;
    bool initiator;
    pan_role_enum pan_role;
    pan_state_t pan_state;
    uint8 device_id;
    pan_callback_t pan_callback;
    uint32 remote_nap_supported_proto_size;
    uint16 remote_nap_supported_protos[BT_SDP_PAN_MAX_SUPPORT_NETWORK_PROTOCOLS];
    uint16 remote_nap_bnep_version;
    uint16 remote_nap_pan_version;
    uint16 remote_nap_security_description;
    uint16 remote_nap_net_access_types;
    uint32 remote_nap_max_net_access_rate;
};

int pan_send_data_sem_wait(void);

void pan_send_data_event_ind(void);

void pan_send_data_done(void);

int pan_can_send_data(void);

void pan_stack_init(struct pan_control_t *(*cb)(uint8_t device_id));

pan_state_t pan_get_state(struct pan_control_t *pan_ctl);

bool pan_channel_is_connected(struct pan_control_t *pan_ctl);

void pan_control_init(struct pan_control_t *pan_ctl, btif_pan_callback_t pbap_callback);

int pan_client_open_req(uint8_t device_id, const void *remote);

int pan_client_close_req(struct pan_control_t *pan_ctl);

int pan_send_ethernet_data(struct pan_control_t *pan_ctl, const struct bnep_ethernet_data_info *data);

#ifdef __cplusplus
}
#endif

#endif /* BT_PAN_SUPPORT */

#endif /* __PAN_I_H__ */

